#include <iostream>
#include <string>
#include <cstring>
#include "IvcReaderWriter.hpp"
//#include "IvcMetaAccess.hpp"
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <map>
#include <fstream>
#include <thread>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <condition_variable>

#define MAX_THREADS_SUPPORTED 1024

volatile bool killThreads[MAX_THREADS_SUPPORTED] = {false};

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
    long long int dataCount;
    long long int ackCount;
    long long int size;
}bufInfo;

Semaphore smphSignalCallbackToPush[MAX_THREADS_SUPPORTED];
std::map<std::string, bufInfo> writeInfo;
std::map<std::string, bufInfo> readInfo;
int processCount = 1;
bool file_write_flag = 0;
std::ofstream file;
ssize_t dataSize = 100;
bool verify_flag = 0;
ssize_t dataCount = -1;
bool debug = false;
bool tput = false;
bool sametopic = false;

// Util Functions. //

void help_function()
{
    std::cout << " '-type | -t'   - this option is to select reader or writer\n";
    std::cout << "                -type w --> to select the writer \n";
    std::cout << "                 -type r --> to select the reader \n\n";
    std::cout << " '-count | -c'  - this option is to give the count of readers/writers selected\n";
    std::cout << "                  - count 2 --> to select 2 writers/readers \n\n";
    std::cout << " '-topic | -tp' - this option is to give the name of the topic\n";
    std::cout << "                 -topic topic_name --> to set topic name as topic_name \n\n";
    std::cout << " '-data | -d' - this option is to give the data if selected as writer\n";
    std::cout << "               -data some_data --> to set data as some_data for writer to write \n\n";
    std::cout << " '-dataCount | -dc' - this option is to give the data count\n";
    std::cout << "               -dataCount count --> to set number of data count \n\n";
    std::cout << " '-f' - this option is to write all the data from terminal to a file\n";
    std::cout << "               -f name.txt --> will write the logs to the name.txt \n\n";
    std::cout << " '-bufsize | -bs' - this option is to generate the data if selected as writer or for the knowledge of the reader if reader option is selected\n";
    std::cout << "               -bufsize N --> to generate random alpha-numeric data of length n for writer to write \n\n";
    std::cout << " '-debug' - this option is to print data from received by reader\n";
    std::cout << "               -debug  --> to make reader print data \n\n";
    std::cout << " '-sametopic' - this option is to make readers/writers created on the same topic. Without this option, readers/writer on incremental topic i.e. topic0, topic1... will be created\n";
    std::cout << "               -sametopic  --> to make readers/writers created on the same topic \n\n";
    std::cout << " '-tput' - this option is to print throughput result by reader\n";
    std::cout << "               -tput  --> to print throughput result\n\n";
}

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
    if(size)
    {
        oss << std::fixed << std::setprecision(2) << size << " " << units[i];
    }
    else
    {
        oss << "0B";
    }
    return oss.str();
}

std::string generateRandom(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
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
        std::cout << "Write Count  : " << it->second.dataCount << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;

        if(file_write_flag == 1)
        {
            file << "Topic: ";
            file << it->first;
            file << " , write count : ";
            file << it->second.dataCount << "\n";
            file << " , ack count : ";
            file << it->second.ackCount << "\n";
        }

        ++it;
    }
    it = readInfo.begin();
    while (it != readInfo.end())
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        if(sametopic == false)
        {
            std::cout << "Reader Topic : " << it->first << std::endl;
            std::cout << "Read Count   : " << it->second.dataCount << std::endl;
        }
        else
        {
            std::cout << "Reader Topic : " << it->first << std::endl;
            std::cout << "Read Count   : " << it->second.dataCount / processCount << std::endl;
        }
        if(file_write_flag == 1)
        {
            file << "Topic: ";
            file << it->first;
            file << " , read count : ";
            file << it->second.dataCount << "\n";
        }
        if(true == tput)
        {
            std::cout << "Topic : " << it->first << " with Average Throughput : " << formatBytes(it->second.size / 10) << " per sec" << std::endl;
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
            std::cout << "passed to decrement user count" << std::endl;
            return 0;
        }
    }
    std::cout << "Failed to decrement user count" << std::endl;
    return 1;
}
#endif

bool readCallBack(readerObj &obj) {
    const char *endOfT = "ByeBye!!";

    if(false == tput)
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
    readInfo[topic].dataCount += 1;
    readInfo[topic].size += obj.bufSize;
    if(true == debug)
    {
        std::cout << "Data read = " << (char*)obj.buffer << std::endl;
    }
    if(file_write_flag == 1) {
        file << (char*)obj.buffer <<"\n";
        std::cout << "Data read = " << (char*)obj.buffer << std::endl;
    }
    if(verify_flag == 1)
    {
        char *temp = (char*)obj.buffer;
        std::cout << "Data read1 = " << temp[0] << std::endl;
        std::cout << "Data read2 = " << temp[1] << std::endl;
        std::cout << "Data read3 = " << temp[obj.bufSize-1] << std::endl;
        std::cout << "Data read4 = " << temp[obj.bufSize-2] << std::endl;
    }
    if((dataCount > 0) && (readInfo[topic].dataCount > dataCount))
    {
        //handleExit();
        killThreads[*((int *)(obj.arg))] = true;
    }

    return true;
}

void launchReader(int rdNo, std::string topic) {
    std::cout << "Launching reader with topic : " << topic << ", with ID = " << rdNo << std::endl;
    readInfo[topic].dataCount = 0;
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
    std::cout << "Write call back triggered: status: "<< obj.cbType << std::endl;
    std::string topic = obj.shMemName;
    if(IVC_READERWRITER_READ_COMPLETED == obj.cbType)
        writeInfo[topic].ackCount += 1;
        smphSignalCallbackToPush[*((int *)(obj.arg))].release();
    return true;
}

void launchWriter(int wrtNo, std::string topic, std::string data) {
    std::cout << "Launching writer with topic : " << topic << ", with ID = " << wrtNo << std::endl;
    const char *endOfT = "ByeBye!!";
    writeInfo[topic].dataCount = 0;
    writeInfo[topic].ackCount = 0;
    writeInfo[topic].size = 0;

    cIvcReaderWriter *obj = new cIvcReaderWriter();
    if(obj && !obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, topic, dataSize, NULL, &writeCallBack, &wrtNo))
    {
        if(!obj->startInterface())
        {
#ifdef ACRN_ENABLE_IVCHEARTBEAT
    incrementUserCount(topic);
#endif
            while (!killThreads[wrtNo]) {
                writeInfo[topic].dataCount += 1;
                const uint8_t *message = reinterpret_cast<const uint8_t*>(&data[0]);
                if(file_write_flag == 1)
                file << message <<"\n";
                if(verify_flag == 1)
                {
                    data[0] = 'A';
                    data[1] = 'A';
                    data[data.length()-1] = 'Z';
                    data[data.length()-2] = 'Z';
                }
                obj->pushData(message, data.length());
                smphSignalCallbackToPush[wrtNo].acquire();
                if((dataCount > 0) && (writeInfo[topic].dataCount > dataCount))
                {
                    break;
                }
                //sleep(1);
            }
            obj->pushData(endOfT, sizeof(endOfT)); //Send end of transmission packet.
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
        std::cout << "setInterfaceConfig failed: obj:" << obj << std::endl;
    }
    delete obj;
    std::cout << "Exiting from Writer-" + std::to_string(wrtNo) << std::endl;
    return;
}

int main(int argc, char* argv[]) {

    std::string readerwriter = "w";
    std::string topic = "topic-";
    std::string data = "";
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
        }else if (strcmp(argv[i], "-topic") == 0 || strcmp(argv[i], "-tp") == 0) {
            topic = argv[i+1];
            i++;
        }else if (strcmp(argv[i], "-data") == 0 || strcmp(argv[i], "-d") == 0) {
            data = argv[i+1];
            i++;
        }else if (strcmp(argv[i], "-dataCount") == 0 || strcmp(argv[i], "-dc") == 0) {
            std::string arg = argv[i+1];
            dataCount = static_cast<ssize_t>(std::stoul(arg, &idx, 10));
            if (idx != arg.length()  || dataCount <= 0) {
                std::cerr << "Invalid bufcount: " << argv[i+1] << std::endl;
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            file_write_flag = 1;
            char *filename = argv[i+1];
            file.open(filename);
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
        else if (strcmp(argv[i], "-verify") == 0) {
            verify_flag = 1;
        }else if (strcmp(argv[i], "-debug") == 0) {
            debug = true;
        }else if (strcmp(argv[i], "-sametopic") == 0) {
            sametopic = true;
        }else if (strcmp(argv[i], "-tput") == 0) {
            tput = true;
        }else {
            std::cout << "invalid option please check --help.\n";
            return 0;
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
            std::cout << "Starting Writer on topic : " << topic << ", with length: " << dataSize;
            if(!data.length())
            {
                data = generateRandom(dataSize);
            }
            if (file_write_flag == 1)
                std::cout << " with data: " << data;
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
        if (processThreads[i].joinable()){
            processThreads[i].join();
            std::cout << "Thread " << i << " joined.\n";
        }
    }
    handleExit();
    std::cout << "Exiting Main\n";
    std::cout << "bye linux\n";
    return 0;
}
