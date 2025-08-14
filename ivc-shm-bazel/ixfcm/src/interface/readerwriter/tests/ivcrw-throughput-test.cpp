/*
    This is a test app for checking the rough throughput of the system. It calculates the amount of data it can pass successfully from a publisher to a subscriber in approximately 10s.
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
#include <sstream>
#include <iomanip>

#define TEST_DURATION 10 //In seconds
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

typedef struct
{
    long long int count;
    long long int size;
}bufInfo;

std::map<std::string, bufInfo> writeInfo;
std::map<std::string, bufInfo> readInfo;
Semaphore smphSignalCallbackToPush[MAX_THREADS_SUPPORTED];
ssize_t dataSize = 100;
bool debug = false;
int processCount = 1;
bool sametopic = false;
bool randomSize = false;
volatile bool killThreads[MAX_THREADS_SUPPORTED] = {false};

// Util Functions. //

//NOTE: formatBytes borrowed from internet
std::string formatBytes(uint64_t bytes) {
    std::string units[] = {"b", "Kb", "Mb", "Gb", "Tb"};
    int i = 0;
    double size = bytes*8;
    while (size >= 1024 && i < sizeof(units)/sizeof(units[0])) {
        size /= 1024;
        i++;
    }
    std::ostringstream oss;
    if(size)
    {
        oss << std::fixed << std::setprecision(2) << size << " " << units[i];
    }
    else
    {
        oss << "0b";
    }
    return oss.str();
}

void help_function()
{
    std::cout << " '-type | -t'   - this option is to select reader or writer\n";
    std::cout << "                -type w --> to select the writer \n";
    std::cout << "                 -type r --> to select the reader \n\n";
    std::cout << " '-count | -c'  - this option is to give the count of readers/writers selected\n";
    std::cout << "                  - count 2 --> to select 2 writers \n\n";
    std::cout << " '-topic | -tp' - this option is to give the name of the topic\n";
    std::cout << "                 -topic topic_name --> to set topic name as topic_name \n\n";
    std::cout << " '-debug | -d'  - this option is to print data additional debug logs\n";
    std::cout << "               -debug  --> debug logs \n\n";
    std::cout << " '-bufsize | -bs' - this option is to generate the data if selected as writer or for the knowledge of the reader if reader option is selected\n";
    std::cout << "               -bufsize N --> to create buffer of length N for writer to write \n\n";
    std::cout << " '-sametopic | -st' - this option is to make readers/writers created on the same topic. Without this option, readers/writer on incremental topic i.e. topic0, topic1... will be created\n";
    std::cout << "               -sametopic  --> to make readers/writers created on the same topic \n\n";
}

////////////////////////////////////////////////////////////////////


void handleExit()
{
    //killThreads = true;
    std::map<std::string, bufInfo>::iterator it = writeInfo.begin();

    while (it != writeInfo.end())
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Writer Topic : " << it->first << std::endl;
        std::cout << "Throughput test duration\t: " << TEST_DURATION << " seconds" << std::endl;
        std::cout << "Write Count\t\t\t: " << it->second.count << std::endl;
        std::cout << "Average Data Chunk Size\t\t: " << formatBytes(it->second.count ? it->second.size/it->second.count : 0) << std::endl;
        std::cout << "Average Throughput\t\t: " << formatBytes((it->second.size) / TEST_DURATION ) << " per sec" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        ++it;
    }
    it = readInfo.begin();
    while (it != readInfo.end())
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        if(sametopic == false)
        {
            std::cout << "READER: Topic: " << it->first << std::endl;
            std::cout << "Read Count  : " << it->second.count << std::endl;
            std::cout << "Topic : " << it->first << " with Average Throughput : " << formatBytes(it->second.size / TEST_DURATION) << " per sec" << std::endl;
        }
        else
        {
            std::cout << "READER: Topic: " << it->first << std::endl;
            std::cout << "Read Count  : " << it->second.count / processCount << std::endl;
            std::cout << "Topic : " << it->first << " with Average Throughput : " << formatBytes((it->second.size / processCount) / TEST_DURATION) << " per sec" << std::endl;
        }
        std::cout << "----------------------------------------------------------" << std::endl;
        ++it;
    }
}

void signal_callback_handler(int signum) {
    std::cout << "######################################################################\n"
              << "                         Caught signal\n"
              << signum
              << "######################################################################\n"
              << std::endl;
    //handleExit();
    //killThreads = true;
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
        return true;
    }
    std::string topic = obj.shMemName;
    readInfo[topic].count += 1;
    readInfo[topic].size += obj.bufSize;

    return true;
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
                sleep(1);
            }
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
    return true;
}

void launchWriter(int wrtNo, std::string topic, void* data) {
    auto start = std::chrono::high_resolution_clock::now();
    const char *endOfT = "ByeBye!!";
    const uint8_t *message = reinterpret_cast<const uint8_t*>(data);
    ssize_t writeSize = dataSize;

    std::cout << "Launching writer with topic : " << topic << ", with ID = " << wrtNo << std::endl;

    writeInfo[topic].count = 0;
    writeInfo[topic].size = 0;

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
    start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - start ) < std::chrono::seconds(TEST_DURATION) ) {
        if(true == randomSize)
        {
            writeSize = (rand() % dataSize) + 1;
        }
        obj->pushData(message, writeSize);
        writeInfo[topic].count += 1;
        writeInfo[topic].size += writeSize;
        smphSignalCallbackToPush[wrtNo].acquire();
    }
    obj->pushData(endOfT, sizeof(endOfT)); //Send end of transmission packet.
#ifdef ACRN_ENABLE_IVCHEARTBEAT
    decrementUserCount(topic);
#endif
    obj->stopInterface();

fail:
    delete obj;
    //handleExit();
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
                std::cout << "Min process count: 1 and Max process count supported: " << MAX_THREADS_SUPPORTED << std::endl;
                return -1;
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
        }else if (strcmp(argv[i], "-topic") == 0 || strcmp(argv[i], "-tp") == 0) {
            topic = argv[i+1];
            i++;
        }else if (strcmp(argv[i], "-sametopic") == 0 || strcmp(argv[i], "-st") == 0) {
            sametopic = true;
        }else if (strcmp(argv[i], "-debug") == 0 || strcmp(argv[i], "-d") == 0) {
            debug = true;
        }
        else if (strcmp(argv[i], "-randomSize") == 0 || strcmp(argv[i], "-rs") == 0) {
            randomSize = true;
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
            for (int i = 0; i < processCount; i++) {
                newTopic = (sametopic == true) ? topic : topic + std::to_string(i);
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
            std::cout << "Throughput Test Started.. Please wait for " << TEST_DURATION << " secs" << std::endl;
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
    free(data);
    handleExit();
    std::cout << "Exiting Main\n";
    return 0;
}
