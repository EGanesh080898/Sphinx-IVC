/*
    This is a test app for checking the rough througput of the system. It calculates the amount of data it can pass successfully from a publisher to a subscriber in approximately 10s.
*/

#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include "IvcReaderWriter.hpp"
//#include "IvcMetaAccess.hpp"
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <condition_variable>
#include <chrono>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>

#define MAX_THREADS_SUPPORTED 1024

class Semaphore {

    private:
        std::mutex mtx;
        std::condition_variable cv;
        int32_t count;

    public:
        Semaphore (int32_t count_ = 0) : count(count_) { }

//        inline void release() {
        void release() {
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }

//        inline void acquire() {
        void acquire() {
            std::unique_lock<std::mutex> lock(mtx);
//            while(count == 0) {
//               cv.wait(lock);
//           }
            cv.wait(lock, [this] { return count > 0; });
            count--;
        }
};

volatile bool killThreads[MAX_THREADS_SUPPORTED] = {false};
typedef struct
{
    int32_t count;
    long long int size;
    std::chrono::microseconds time;
}bufInfo;

std::map<std::string, bufInfo> writeInfo;
std::map<std::string, bufInfo> readInfo;
Semaphore smphSignalCallbackToPush[MAX_THREADS_SUPPORTED];
ssize_t dataSize = 100;
ssize_t dataCount = 11;
bool debug = false;
bool sametopic = false;
int processCount = 1;

// Util Functions. //

//NOTE: formatBytes borrowed from internet
std::string formatBytes(uint64_t bytes) {
    std::string units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size = bytes;
    while (size >= 1024 && i < sizeof(units)/sizeof(units[0])) {
        size /= 1024;
        i++;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[i];
    return oss.str();
}

void help_function()
{
    std::cout << " '-type' - this option is to select reader or writer\n";
    std::cout << "                -type w --> to select the writer \n";
    std::cout << "                 -type r --> to select the reader \n\n";
    std::cout << " '-count' - this option is to give the count of writersselected\n";
    std::cout << "                  - count 2 --> to select 2 writers \n\n";
    std::cout << " '-topic' - this option is to give the name of the topic\n";
    std::cout << "                 -topic topic_name --> to set topic name as topic_name \n\n";
    std::cout << " '-debug' - this option is to print data additional debug logs\n";
    std::cout << "               -debug  --> debug logs \n\n";
    std::cout << " '-bufsize' - this option is to generate the data if selected as writer or for the knowledge of the reader if reader option is selected\n";
    std::cout << "               -bufsize N --> to create buffer of length N for writer to write \n\n";
    std::cout << " '-bufcount' - this option is give number of buffer count after which average latency is calculated\n";
    std::cout << "               -bufcount C --> buffer count after which average latency is calculated \n\n";
    std::cout << " '-sametopic' - this option is to make readers/writers created on the same topic. Without this option, readers/writer on incremental topic i.e. topic0, topic1... will be created\n";
    std::cout << "               -sametopic  --> to make readers/writers created on the same topic \n\n";
}

////////////////////////////////////////////////////////////////////

void handleExit()
{
    //killThreads = true;
    int32_t rc = 0;
    int32_t wc = 0;
    for(auto& ri : readInfo)
    {
        rc++;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "READER: " << rc << std::endl;
        if(sametopic == false)
            std::cout << "READER: Topic: " << ri.first << ", Packet count received: " << ri.second.count << std::endl;
        else
            std::cout << "READER: Topic: " << ri.first << ", Packet count received: " << ri.second.count / processCount << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
    }

    for (auto& wi : writeInfo)
    {
        wc++;
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "WRITER: " << wc << std::endl;
        std::cout << "Topic : " << wi.first << std::endl;
        std::cout << "Latency test " << std::endl;
        std::cout << "Packet count sent: " << wi.second.count << std::endl;
        std::cout << "Average Latency\t\t: " << std::chrono::duration_cast<std::chrono::microseconds>(wi.second.time).count() / wi.second.count << " microseconds" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
    }
}

void signal_callback_handler(int signum) {
    std::cout << "######################################################################\n"
              << "                         Caught signal\n"
              << signum
              << "######################################################################\n"
              << std::endl;
    //handleExit();
    for(int i=0;i<processCount;i++)
        killThreads[i] = true;
    std::cout << "Exiting signal Handler.\n";
    return;
}

#ifdef ACRN_ENABLE_IVCHEARTBEAT
int incrementUserCount(std::string &topic)
{
    cIvcMetaAccess *metaObj = new cIvcMetaAccess();

    if(metaObj)
    {
        if(!metaObj->attachToMemory(topic))
        {
            metaObj->incrementUserCount();
            metaObj->detachFromMemory();
            delete metaObj;
            return 0;
        }
    }
    std::cout << "Failed to increment user count" << std::endl;
    return 1;
}

int decrementUserCount(std::string &topic)
{
    cIvcMetaAccess *metaObj = new cIvcMetaAccess();

    if(metaObj)
    {
        if(!metaObj->attachToMemory(topic))
        {
            metaObj->decrementUserCount();
            metaObj->detachFromMemory();
            delete metaObj;
            return 0;
        }
    }
    std::cout << "Failed to increment user count" << std::endl;
    return 1;
}
#endif

bool readCallBack(readerObj &obj) {
    const char *endOfT = "ByeBye!!";

    if(true == debug)
    {
        std::cout << "Read call back triggered with event: " << obj.cbType << " and buffer size: "<< obj.bufSize << std::endl;
    }
    if((obj.bufSize == strlen(endOfT)) && !strncmp((char*)obj.buffer, endOfT, strlen(endOfT)))
    {
        //handleExit();
        killThreads[*((int *)(obj.arg))] = true;
        return 0;
    }
    std::string topic = obj.shMemName;
    readInfo[topic].count += 1;
    readInfo[topic].size += obj.bufSize;

    return 0;
}


void launchReader(int rdNo, std::string topic) {
    std::cout << "Launching reader with topic : " << topic << ", with ID = " << rdNo << std::endl;
    readInfo[topic].count = 0;
    readInfo[topic].size = 0;
    cIvcReaderWriter *obj = new cIvcReaderWriter();
    if(!obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, topic, dataSize, &readCallBack, NULL, &rdNo))
    {
        if(!obj->startInterface())
        {
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            incrementUserCount(topic);
#endif
            while (!killThreads[rdNo]) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            decrementUserCount(topic);
#endif
            obj->stopInterface();
            //handleExit();
        }
        else
        {
            std::cout << "startInterface failed" << std::endl;
        }
    }
    else
    {
        std::cout << "setInterfaceConfig failed" << std::endl;
    }
    delete obj;
    std::cout << "Exiting from Reader " + std::to_string(rdNo) << std::endl;
    return;
}

bool writeCallBack(writerObj &obj) {
    if(true == debug)
    {
        std::cout << "Write call back triggered: status: "<< obj.cbType << std::endl;
    }
    smphSignalCallbackToPush[*((int *)(obj.arg))].release();
    //if(IVC_READERWRITER_READ_COMPLETED == obj.cbType)
    //    writeCount++;
    return 0;
}

void launchWriter(int wrtNo, std::string topic, void* data) {
    writeInfo[topic].count = dataCount;
    writeInfo[topic].size = 0;
    ssize_t dc = dataCount;
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    const char *endOfT = "ByeBye!!";
    std::chrono::microseconds time(0);
    std::cout << "Launching writer with topic : " << topic << ", with ID = " << wrtNo << std::endl;
    cIvcReaderWriter *obj = new cIvcReaderWriter;
    if(!obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, topic, dataSize, NULL, &writeCallBack, &wrtNo))
    {
        if(obj->startInterface())
        {
            std::cout << "startInterface failed" << std::endl;
            goto fail;
        }
    }
    else
    {
        std::cout << "setInterfaceConfig failed" << std::endl;
        goto fail;
    }
 #ifdef ACRN_ENABLE_IVCHEARTBEAT
    incrementUserCount(topic);
#endif
    while (!killThreads[wrtNo] && dc) {
        start = std::chrono::high_resolution_clock::now();
        const uint8_t *message = reinterpret_cast<const uint8_t*>(data);
        obj->pushData(message, dataSize);
        smphSignalCallbackToPush[wrtNo].acquire();
        end = std::chrono::high_resolution_clock::now();
        diff = std::chrono::duration_cast<std::chrono::microseconds>( end - start );
        dc--;
        writeInfo[topic].size += dataSize;
        writeInfo[topic].time += diff;
        if(true == debug)
        {
            std::cout << "time " << std::chrono::duration_cast<std::chrono::microseconds>(writeInfo[topic].time).count() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    obj->pushData(endOfT, sizeof(endOfT)); //Send end of transmission packet.
#ifdef ACRN_ENABLE_IVCHEARTBEAT
    decrementUserCount(topic);
#endif

    obj->stopInterface();
fail:
    delete obj;
    std::cout << "\nExiting from Writer-" + std::to_string(wrtNo) << std::endl;
    return;
}

int main(int argc, char* argv[]) {
    std::string readerwriter = "w";
    std::string topic = "topic-";
    void* data = NULL;
    std::map<std::string, int> command_map;
    command_map["r"] = 1;
    command_map["w"] = 2;
    size_t idx = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0) {
            help_function();
            return 0;
        }
        else if (strcmp(argv[i], "-type") == 0 || strcmp(argv[i], "-t") == 0) {
            readerwriter = argv[i+1];
            i++;
        }
        else if (strcmp(argv[i], "-count") == 0 || strcmp(argv[i], "-c") == 0) {
            std::string arg = argv[i+1];
            processCount = static_cast<int32_t>(std::stoi(arg, nullptr, 10));
            if(processCount >= MAX_THREADS_SUPPORTED || processCount <= 0)
            {
                std::cerr << "Invalid count: " << argv[i+1] << std::endl;
                std::cerr << "Min process count: 1 and Max process count supported: " << MAX_THREADS_SUPPORTED << std::endl;
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-bufsize") == 0 || strcmp(argv[i], "-bs") == 0) {
            std::string arg = argv[i+1];
            dataSize = static_cast<ssize_t>(std::stoul(arg, &idx, 10));
            if (idx != arg.length() || dataSize <= 0) {
                std::cerr << "Invalid bufsize: " << argv[i+1] << std::endl;
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-bufcount") == 0) {
            std::string arg = argv[i+1];
            dataCount = static_cast<ssize_t>(std::stoul(arg, &idx, 10));
            if (idx != arg.length()  || dataCount <= 0) {
                std::cerr << "Invalid bufcount: " << argv[i+1] << std::endl;
                return 1;
            }
            i++;
        }else if (strcmp(argv[i], "-topic") == 0 || strcmp(argv[i], "-tp") == 0) {
            topic = argv[i+1];
            i++;
        }else if (strcmp(argv[i], "-sametopic") == 0) {
            sametopic = true;
        }else if (strcmp(argv[i], "-debug") == 0) {
            debug = true;
        }
        else {
            std::cout << "invalid option " << argv[i] << " please check --help.\n";
            return -1;
        }
    }

    std::thread processThreads[processCount];
    std::string newTopic;

    switch (command_map[readerwriter])
    {
        case 1:
        {
            std::cout << "Starting Reader on topic : " << topic << ", with length: " << dataSize << std::endl;
            std::cout << "Latency Test. Waiting for Writer.. " << std::endl;

            for (int i = 0; i < processCount; i++) {
                newTopic = (sametopic == true) ? topic : topic + std::to_string(i);
                //writeInfo
                processThreads[i] = std::thread(launchReader, i, newTopic);
            }
            break;
        }
        case 2:
        {
            std::cout << "Starting Writer on topic : " << topic << ", with length: " << dataSize << std::endl;
            data = malloc(dataSize);

            if (data == NULL)
                return -1;

            memset(data, 0xA5A5A5A5, dataSize);

            std::cout << "Latency Test Started.. Please wait.. " << std::endl;
            std::cout << "\n";

            for (int i = 0; i < processCount; i++) {
                newTopic = (sametopic == true) ? topic : topic + std::to_string(i);
                processThreads[i] = std::thread(launchWriter, i, newTopic, data);
            }
            break;
        }
        default:
        {
            std::cerr << "Invalid Options!! \n";
            std::cout << "-type w --> to select the writer \n";
            std::cout << "-type r --> to select the reader \n";
            return 0;
        }
    }

    signal(SIGINT, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);

    for (int i = 0; i < processCount; i++) {
        if (processThreads[i].joinable()) {
            processThreads[i].join();
            std::cout << "Thread " << i << " joined.\n";
        }
    }
    if(data) free(data);

    handleExit();
    std::cout << "Exiting Main\n";
    return 0;
}
