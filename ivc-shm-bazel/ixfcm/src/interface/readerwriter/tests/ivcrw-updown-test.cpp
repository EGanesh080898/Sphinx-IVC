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

#define MAX_THREADS_SUPPORTED 1024

volatile bool killThreads = false;
std::map<std::string, long long int> writeCount;
std::map<std::string, long long int> readCount;
bool file_write_flag = 0;
std::ofstream file;
ssize_t dataSize = 100;
int processCount = 1;

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
    std::cout << " '-bufsize | -bs' - this option is to give the size of shared memory\n";
    std::cout << "               -bufsize some_size --> to set shared memory size \n\n";
    std::cout << " '-msgCount' - this option is to give the number of messages before the thread is killed\n";
}

////////////////////////////////////////////////////////////////////

void signal_callback_handler(int signum) {
    std::cout << "Caught signal " << signum << std::endl;
    killThreads = true;
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
    std::string topic = obj.shMemName;
    readCount[topic] += 1;
    std::cout << "Read call back triggered with event: " << obj.cbType <<std::endl;
    if(file_write_flag == 1) {
        std::cout << "Data read = " << obj.buffer << std::endl;
        file << obj.buffer <<"\n";
    }

    return true;
}

bool writeCallBack(writerObj &obj) {
    std::cout << "Write call back triggered: status: "<< obj.cbType << std::endl;
    return true;
}

void launchReader(int rdNo, std::string topic, int msgCount) {
    std::cout << "Launching reader with topic : " << topic << ", with ID = " << rdNo << std::endl;
    if (readCount.find(topic) == readCount.end())
         readCount[topic] = 0;
    cIvcReaderWriter *obj = new cIvcReaderWriter();
    if(!obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, topic, dataSize, &readCallBack, NULL, NULL))
    {
        if(!obj->startInterface())
        {
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            incrementUserCount(topic);
#endif
            while (!killThreads && msgCount--) {
                sleep(1);
            }
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            decrementUserCount(topic);
#endif
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
    std::cout << "Exiting from Reader " + std::to_string(rdNo) << std::endl;
    return;
}

void launchWriter(int wrtNo, std::string topic, std::string data, int msgCount) {
    std::cout << "Launching writer with topic : " << topic << ", with ID = " << wrtNo << std::endl;
    //long long int count = 0;
    ssize_t writesize = dataSize;
    if (writeCount.find(topic) == writeCount.end())
         writeCount[topic] = 0;
    cIvcReaderWriter *obj = new cIvcReaderWriter();
    std::string newData = data + (std::to_string(writeCount[topic]) + " : from writer-" + std::to_string(wrtNo) + " in topic " + topic + ".");
    const uint8_t *message = reinterpret_cast<const uint8_t*>(&newData[0]);
    if(file_write_flag == 1)
        file << message <<"\n";
    if(writesize > newData.length())
        writesize = newData.length();
    if(!obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, topic, dataSize, NULL, &writeCallBack, NULL))
    {
        if(!obj->startInterface())
        {
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            incrementUserCount(topic);
#endif
            while (!killThreads && msgCount--) {
                writeCount[topic] += 1;
                obj->pushData(message, writesize);
                sleep(1);
            }
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            decrementUserCount(topic);
#endif
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
    std::cout << "Exiting from Writer-" + std::to_string(wrtNo) << std::endl;
    return;
}

int main(int argc, char* argv[]) {
    std::string pubSub = "w";
    std::string topic = "topic-";
    std::string data = "Message ";
    ssize_t msgCount = 5;
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
            pubSub = argv[i + 1];
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
            topic = argv[i + 1];
            i++;
        }else if (strcmp(argv[i], "-data") == 0 || strcmp(argv[i], "-d") == 0) {
            data = argv[i +1];
            i++;
        }else if (strcmp(argv[i], "-bufsize") == 0 || strcmp(argv[i], "-bs") == 0) {
            std::string arg = argv[i+1];
            dataSize = static_cast<ssize_t>(std::stoul(arg, &idx, 10));
            if (idx != arg.length() || dataSize <= 0) {
                std::cerr << "Invalid bufsize: " << argv[i+1] << std::endl;
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-msgCount") == 0) {
            std::string arg = argv[i+1];
            msgCount = static_cast<ssize_t>(std::stoul(arg, &idx, 10));
            if (idx != arg.length()  || msgCount <= 0) {
                std::cerr << "Invalid bufcount: " << argv[i+1] << std::endl;
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-f") == 0) {
            file_write_flag = 1;
            char *filename = argv[i+1];
            file.open(filename);
            i++;
        }
        else {
            std::cout << "invalid option please check --help.\n";
            return 0;
        }
    }

    std::thread processThreads[processCount];

    signal(SIGINT, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);

    int detachCount = 0;

    while (!killThreads) {
        switch (command_map[pubSub]) {
            case 1:
            {
                std::cout << "Starting Reader on topic : " << topic << std::endl;
                for (int i = 0; i < processCount; i++) {
                    std::string newTopic = topic + std::to_string(i);
                    processThreads[i] = std::thread(launchReader, i, newTopic, msgCount + ( rand() % ( (msgCount * processCount) - msgCount + 1 ) ) );
                }
                break;
            }
            case 2:
            {
                std::cout << "Starting Writer on topic : " << topic;
                if (file_write_flag == 1)
                    std::cout << " with data: " << data;
                std::cout << "\n";

                for (int i = 0; i < processCount; i++) {
                    std::string newTopic = topic + std::to_string(i);
                    processThreads[i] = std::thread(launchWriter, i, newTopic, data, msgCount);
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
        for (int i = 0; i < processCount; i++) {
            if (processThreads[i].joinable()) {
                processThreads[i].join();
                detachCount += 1;
                std::cout << "Thread " << i << " joined.\n";
            }
        }
    }

    std::map<std::string, long long int>::iterator it = writeCount.begin();
    while (it != writeCount.end())
    {
      std::cout << std::endl;
      std::cout << "----------------------------------------------------------" << std::endl;
      std::cout << "Topic: " << it->first << ", Count: " << it->second << std::endl;
      if(file_write_flag == 1) {
        file << "Topic: ";
        file << it->first;
        file << " , write count : ";
        file << it->second << "\n";
      }
      std::cout << std::endl;
      std::cout << "----------------------------------------------------------" << std::endl;
      ++it;
    }
    it = readCount.begin();
    while (it != readCount.end())
    {
      std::cout << std::endl;
      std::cout << "----------------------------------------------------------" << std::endl;
      std::cout << "Topic: " << it->first << ", Count: " << it->second << std::endl;
      if(file_write_flag == 1) {
        file << "Topic: ";
        file << it->first;
        file << " , read count : ";
        file << it->second << "\n";
      }
      std::cout << std::endl;
      std::cout << "----------------------------------------------------------" << std::endl;
      ++it;
    }

    std::cout << "Detach Count : " << detachCount << std::endl;
    std::cout << "Exiting Main\n";

    return 0;
}
