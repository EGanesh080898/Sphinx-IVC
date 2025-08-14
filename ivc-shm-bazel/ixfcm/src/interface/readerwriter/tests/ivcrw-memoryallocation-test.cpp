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
#include <vector>

#define MAX_SHARED_MEMORY_SIZE 524200000
#define MAX_THREADS_SUPPORTED 1024

ssize_t maxShmemSize = 262144000;

volatile bool killThreads[MAX_THREADS_SUPPORTED] = {false};


typedef struct
{
    std::string topic;
    ssize_t size;
    bool status;
}iterInfo;

std::map<int, std::vector<iterInfo>> allInfo;
int processCount = 1;

// Util Functions. //

void help_function()
{
    std::cout << " '-count' - this option is to give the count of writers/readers selected\n";
    std::cout << "                  - count 2 --> to select 2 writers/readers \n\n";
    std::cout << " '-topic' - this option is to give the name of the topic\n";
    std::cout << "                 -topic topic_name --> to set topic name as topic_name \n\n";
    std::cout << " '-maxsize' - this option is to give the maximum size of shared memory available \n";
    std::cout << "                 -maxsize maximum_size_value --> to set maxsize as maximum size_value \n\n";
    std::cout << "                 - run ivcrw-get-shmeminfo test app to know about maximum shared memory \n";
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


void handleExit()
{
    for(auto &tmp: allInfo)
    {
        int pc=0, fc=0;
        for(auto &tmp2: tmp.second)
        {
            std::cout << std::endl;
            std::cout << "----------------------------------------------------------" << std::endl;
            std::cout << "Topic:" << tmp2.topic << ", size: " << formatBytes(tmp2.size) << ", status: " << tmp2.status << std::endl;
            if(tmp2.status == true) pc++; else fc++;
        }
        std::cout << "Pass Count: " << pc << ", Fail Count: " << fc << std::endl;
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
    }
}

void signal_callback_handler(int signum) {
    std::cout << "######################################################################\n"
        << "                         Caught signal\n"
        << "######################################################################\n"
        << signum << std::endl;
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

void launchRWThread(int rdNo, std::string topic)
{
    std::cout << "Launching RW with topic : " << topic << ", with ID = " << rdNo << std::endl;
    std::vector<iterInfo> iterList;
    iterInfo iter;
    iter.topic = topic;
    while (!killThreads[rdNo])
    {
        iter.size = (rand() % maxShmemSize) + 1;
        iter.status = false;
        //sleep(11);
        cIvcReaderWriter *obj = new cIvcReaderWriter();
        //std::cout << "Created\n" << std::endl;
        //sleep(11);
        if(obj)
        {
            if(!obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_SYNC_WRITER, topic, iter.size, NULL, NULL, &rdNo))
            {
                //std::cout << "setInterfaceConfig\n" << std::endl;
                //sleep(11);
                if(!obj->startInterface())
                {
                    iter.status = true;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    //std::cout << "startInterface\n" << std::endl;
                    //sleep(11);
                    obj->stopInterface();
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
            obj = NULL;
        }
        //std::cout << "Deleted\n" << std::endl;
        //sleep(11);
        iterList.push_back(iter);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    allInfo[rdNo] = iterList;

    std::cout << "Exiting from Reader " + std::to_string(rdNo) << std::endl;
    return;
}


int main(int argc, char* argv[])
{
    std::string topic = "topic-";
    size_t idx = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0) {
            help_function();
            return 0;
        }
        else if (strcmp(argv[i], "-count") == 0) {
            std::string arg = argv[i+1];
            processCount = static_cast<int32_t>(std::stoi(arg, nullptr, 10));
            if(processCount >= MAX_THREADS_SUPPORTED || processCount <= 0)
            {
                std::cout << "Min process count: 1 and Max process count supported: " << MAX_THREADS_SUPPORTED << std::endl;
                return -1;
            }
            i++;
        }else if (strcmp(argv[i], "-topic") == 0) {
            topic = argv[i+1];
            i++;
        }
        else if (strcmp(argv[i], "-maxsize") == 0) {
            //maxShmemSize = std::stoi(argv[i+1]);
            std::string arg = argv[i+1];
            maxShmemSize = static_cast<ssize_t>(std::stoul(arg, &idx, 10));
            if (idx != arg.length() || maxShmemSize <= 0) {
                std::cerr << "Invalid bufsize: " << argv[i+1] << std::endl;
                return 1;
            }
            i++;
        }
        else {
            std::cout << "invalid option please check --help.\n";
            return 0;
        }
    }

    std::thread processThreads[processCount];
    std::string newTopic;

    for (int i = 0; i < processCount; i++) {
        newTopic = topic + std::to_string(i);
        processThreads[i] = std::thread(launchRWThread, i, newTopic);
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

