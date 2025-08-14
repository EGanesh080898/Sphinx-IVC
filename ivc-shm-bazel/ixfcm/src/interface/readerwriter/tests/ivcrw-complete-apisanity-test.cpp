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

volatile bool killThreads[MAX_THREADS_SUPPORTED] = {false};
std::map<std::string, long long int> writeCount;
std::map<std::string, long long int> readCount;
int processCount = 1;
bool file_write_flag = 0;
std::ofstream file;
ssize_t dataSize = 100;
int test;
bool verify_flag = 0;
ssize_t dataCount = -1;
bool debug = false;
bool readFlag = false;
bool sametopic = false;

// Util Functions. //

void help_function()
{
    std::cout << " '-type' - this option is to select reader or writer\n";
    std::cout << "                -type w --> to select the writer \n";
    std::cout << "                 -type r --> to select the reader \n\n";
    std::cout << " '-count' - this option is to give the count of writers/readers selected\n";
    std::cout << "                  - count 2 --> to select 2 writers/readers \n\n";
    std::cout << " '-topic' - this option is to give the name of the topic\n";
    std::cout << "                 -topic topic_name --> to set topic name as topic_name \n\n";
    std::cout << " '-data' - this option is to give the data if selected as writer\n";
    std::cout << "               -data some_data --> to set data as some_data for writer to write \n\n";
    std::cout << " '-dataCount' - this option is to give the data count\n";
    std::cout << "               -dataCount count --> to set number of data count \n\n";
    std::cout << " '-f' - this option is to write all the data from terminal to a file\n";
    std::cout << "               -f name.txt --> will write the logs to the name.txt \n\n";
    std::cout << " '-bufsize' - this option is to generate the data if selected as writer or for the knowledge of the reader if reader option is selected\n";
    std::cout << "               -bufsize N --> to generate random alpha-numeric data of length n for writer to write \n\n";
    std::cout << " '-debug' - this option is to print data from received by reader\n";
    std::cout << "               -debug  --> to make reader print data \n\n";
    std::cout << " '-sametopic' - this option is to make readers/writers created on the same topic. Without this option, readers/writer on incremental topic i.e. topic0, topic1... will be created\n";
    std::cout << "               -sametopic  --> to make readers/writers created on the same topic \n\n";
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
    std::map<std::string, long long int>::iterator it = writeCount.begin();

    while (it != writeCount.end())
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Writer Topic : " << it->first << std::endl;
        std::cout << "Write Count  : " << it->second << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        if(file_write_flag == 1)
        {
            file << "Topic: ";
            file << it->first;
            file << " , write count : ";
            file << it->second << "\n";
        }

        ++it;
    }
    it = readCount.begin();
    while (it != readCount.end())
    {
        std::cout << std::endl;
        if(sametopic == false)
        {
            std::cout << "Reader Topic : " << it->first << std::endl;
            std::cout << "Read Count   : " << it->second << std::endl;
        }
        else
        {
            std::cout << "Reader Topic : " << it->first << std::endl;
            std::cout << "Read Count   : " << it->second / processCount << std::endl;
        }
        if(file_write_flag == 1)
        {
            file << "Topic: ";
            file << it->first;
            file << " , read count : ";
            file << it->second << "\n";
        }

        ++it;
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

bool readCallBack(readerObj &obj) {
    std::cout << "Read call back triggered with event: " << obj.cbType <<std::endl;
    if(test == 5 || test == 7)
        killThreads[*((int *)(obj.arg))] = true;
    readFlag = true;
    std::string topic = obj.shMemName;
    readCount[topic] += 1;
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
    if((dataCount > 0) && (readCount[topic] > dataCount))
    {
        killThreads[*((int *)(obj.arg))] = true;
    }

    return 0;
}

bool writeCallBack(writerObj &obj) {
    std::cout << "Write call back triggered: status: "<< obj.cbType << std::endl;
    std::string topic = obj.shMemName;
    if(IVC_READERWRITER_READ_COMPLETED == obj.cbType)
        writeCount[topic] += 1;
    return true;
}

void launchReader(int rdNo, std::string topic) {
    std::cout << "Launching reader2 with topic : " << topic << ", with ID = " << rdNo << std::endl;
    cIvcReaderWriter *obj = new cIvcReaderWriter();

    int32_t retval;
    if(test == 1)
    {
        retval = obj->setInterfaceConfig(LGIVC_SYNC_READER, topic, dataSize, NULL, NULL, &rdNo);
    }
    else if(test == 2)
    {
    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER, topic, dataSize, &readCallBack, NULL, &rdNo);
    }
    else if(test == 3)
    {
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER, topic, dataSize, &readCallBack, NULL, &rdNo);
    }else if(test == 4)
    {
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER, topic, dataSize, NULL, NULL, &rdNo);
    }else if(test == 5)
    {
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, topic, dataSize, &readCallBack, NULL, &rdNo);
    }else if(test == 6)
    {
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, topic, dataSize, &readCallBack, NULL, &rdNo);
    }else if(test == 7)
    {
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, topic, dataSize, &readCallBack, &writeCallBack, &rdNo);
    }else if(test == 8)
    {
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, topic, dataSize, NULL, NULL, &rdNo);
    }
    else if(test == 9)
    {
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, topic, dataSize, &readCallBack, NULL, &rdNo);
    }
    else
    {
        printf("invalid option\n");
        delete obj;
        std::cout << "Exiting from Reader " + std::to_string(rdNo) << std::endl;
        return;
    }
    int cnt = 0;
    if(!retval)
    {
        if(!obj->startInterface())
        {
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            incrementUserCount(topic);
#endif
            uint8_t *message;
            bool ret;
            message = (uint8_t*)malloc(dataSize+1);
            while (!killThreads[rdNo]) {
                sleep(1);
                if(readFlag == true && (test == 6 || test == 9))
                {
                   if(test == 6)
                       ret = obj->pullData(message, dataSize);
                   else
                   {
                       ret = obj->read(message, dataSize);
                       cnt++;
                       if(cnt == 2)
                           killThreads[rdNo] = true;
                   }
                   if(ret == 0)
                   {
                       printf("pulldata/read passed\n");
                       message[dataSize] = '\0';
                       std::cout<<"message = "<<(char*)message<<"\n";
                   }
                   else
                   {
                      printf("pulldata/read failed\n");
                      killThreads[rdNo] = true;
                   }
                }
                else if(test == 1 || test == 2 || test == 3)
                    killThreads[rdNo] = true;
            }
            free(message);
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            decrementUserCount(topic);
#endif
            obj->stopInterface();
            //handleExit();
        }
        else
        {
            std::cout << "startInterface failed" << std::endl;
            delete obj;
            std::cout << "Exiting from Reader-" + std::to_string(rdNo) << std::endl;
            printf("\n\nTest Case Failed!!\n\n");
            return;
        }
    }
    else
    {
        std::cout << "setInterfaceConfig failed" << std::endl;
        delete obj;
        std::cout << "Exiting from Reader-" + std::to_string(rdNo) << std::endl;
        if(test == 3 || test == 4 || test == 8)
        {
            printf("\n\nTest Case Passed!!\n\n");
            return;
        }
        printf("\n\nTest Case Failed!!\n\n");
        return;
    }
    delete obj;
    std::cout << "Exiting from Reader " + std::to_string(rdNo) << std::endl;
    if(test == 3 || test == 4)
    {
        printf("\n\nTest Case Failed!!\n\n");
        return;
    }
    printf("\n\nTest Case Passed!!\n\n");
    return;
}

void launchWriter(int wrtNo, std::string topic, std::string data) {
    std::cout << "Launching writer with topic : " << topic << ", with ID = " << wrtNo << std::endl;
    writeCount[topic] = 0;

    cIvcReaderWriter *obj = new cIvcReaderWriter();
    int32_t retval;
    if(test == 1)
        retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, topic, dataSize, NULL, NULL, &wrtNo);
    else if(test == 2)
        retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, topic, dataSize, NULL, &writeCallBack, &wrtNo);
    else if(test == 3)
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, topic, dataSize, NULL, &writeCallBack, &wrtNo);
    else if(test == 4)
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, topic, dataSize, NULL, NULL, &wrtNo);
    else if(test == 5)
        retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, topic, dataSize, &readCallBack, &writeCallBack, &wrtNo);
    else if(test == 6)
        retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, topic, dataSize, &readCallBack, &writeCallBack, &wrtNo);
    else
    {
        printf("invalid option\n");
        delete obj;
        std::cout << "Exiting from Writer-" + std::to_string(wrtNo) << std::endl;
        return;
    }
    if(!retval)
    {
        if(!obj->startInterface())
        {
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            incrementUserCount(topic);
#endif
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
            printf("writing the data....\n");
            obj->pushData(message, data.length());
            sleep(1);
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            decrementUserCount(topic);
#endif
            obj->stopInterface();
            //handleExit();
        }
        else
        {
            std::cout << "startInterface failed" << std::endl;
            delete obj;
            std::cout << "Exiting from Writer-" + std::to_string(wrtNo) << std::endl;
            printf("\n\nTest Case Failed!!\n\n");
            return;
        }
    }
    else
    {
        std::cout << "setInterfaceConfig failed" << std::endl;
        delete obj;
        std::cout << "Exiting from Writer-" + std::to_string(wrtNo) << std::endl;
        if(test == 4)
        {
            printf("\n\nTest Case Passed!!\n\n");
            return;
        }
        return;
    }
    delete obj;
    std::cout << "Exiting from Writer-" + std::to_string(wrtNo) << std::endl;
    if(test == 4)
    {
        printf("\n\nTest Case Failed!!\n\n");
        return;
    }
    printf("\n\nTest Case Passed!!\n\n");
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
        else if (strcmp(argv[i], "-type") == 0) {
            readerwriter = argv[i+1];
            i++;
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
        }else if (strcmp(argv[i], "-data") == 0) {
            data = argv[i+1];
            i++;
        }else if (strcmp(argv[i], "-dataCount") == 0) {
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
        else if (strcmp(argv[i], "-bufsize") == 0) {
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
        }else {
            std::cout << "invalid option please check --help.\n";
            return 0;
        }
    }

    if(readerwriter == "w")
    {
        printf("Select the below options to test writer\n");
        printf("1. TO test passing LGIVC_SYNC_WRITER without passing write callback in setinterfaceconfig()\n");
        printf("   LGIVC_SYNC_WRITER - when we dont want to trigger writer callback we will use this.\n");
        printf("   Result - This will send the data but it wont trigger writer callback.\n");
        printf("2. TO test passing LGIVC_SYNC_WRITER with passing write callback in setinterfaceconfig()\n");
        printf("   LGIVC_SYNC_WRITER - It doesnot require passing callback, but we are passing callback.\n");
        printf("   Result - Warning message should come like ignoring callback and it should write data.\n");
        printf("3. TO test passing LGIVC_ASYNC_WRITER with passing callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_WRITER - when we want to trigger writer callback we will use this.\n");
        printf("   Result - This will send the data and triggers the writer callback.\n");
        printf("4. TO test passing LGIVC_ASYNC_WRITER without passing callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_WRITER - when we want to trigger writer callback we will use this.\n");
        printf("   Result - This should fail as we are not passing any callback along with it.\n");
        printf("5. TO test passing LGIVC_SYNC_WRITER with passing both read and write callbacks in setinterfaceconfig()\n");
        printf("   LGIVC_SYNC_WRITER - we should not pass any callback to this.\n");
        printf("   Result - Warning message should come like ignoring read and write callbacks and it should write data.\n");
        printf("6. TO test passing LGIVC_ASYNC_WRITER with passing both read and write callbacks in setinterfaceconfig()\n");
        printf("   LGIVC_SYNC_WRITER - we should pass writer callback to this.\n");
        printf("   Result - Warning message should come like ignoring read callback and it should write data and triggers write callback.\n");

    }
    else
    {
        printf("Select the below options to test reader\n");
        printf("\nNOTE :: Run writer from another VM to send data\n\n");
        printf("1. TO test passing LGIVC_SYNC_READER without passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_SYNC_READER - When we dont want to trigger reader callback and not to read data we will use this.\n");
        printf("   Result - Reader thread wont be getting launched and callback should not triggered\n");
        printf("2. TO test passing LGIVC_SYNC_READER with passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_SYNC_READER - It doesnot require passing reaad callback, but we are passing callback.\n");
        printf("   Result - Warning message should come like ignoring read callback and it should not launch reader thread.\n");
        printf("3. TO test passing LGIVC_ASYNC_READER with passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_READER - When we want to trigger reader callback, but we are not passing LGIVC_WITH_DATACALLBACK or LGIVC_WITH_EVENTCALLBACK\n");
        printf("   Result - It should not launch reader thread and also wont trigger reader callback, it should fail\n");
        printf("4. TO test passing LGIVC_ASYNC_READER without passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_READER - When we want to trigger reader callback, but we are not passing LGIVC_WITH_DATACALLBACK or LGIVC_WITH_EVENTCALLBACK\n");
        printf("   Result - It should fail as we are not passingread callback. It requires read callback.\n");
        printf("5. TO test passing LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK with passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK - When we want to trigger reader callback and also data will be read in reader thread automaticaly.\n");
        printf("   Result - It should receive data in reader thread with triggering read callback.\n");
        printf("6. TO test passing LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK with passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK - When we want to trigger reader callback but we are calling pulldata api to receive data\n");
        printf("   Result - It should trigger read callback and by using pulldata same data can be received only once, it will fail reaming time..\n");
        printf("7. TO test passing LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK with passing both read and write callbacks in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK - When we want to trigger reader callback and also data will be read in reader thread automaticaly.\n");
        printf("   Result - It should show warning message like ignoring writer callback and should receive data in reader thread with triggering read callback.\n");
        printf("8. TO test passing LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK without passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK - When we want to trigger reader callback and also data will be read in reader thread automaticaly.\n");
        printf("   Result - It should fail as we are not passingread callback. It requires read callback.\n");
        printf("9. TO test passing LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK with passing read callback in setinterfaceconfig()\n");
        printf("   LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK - When we want to trigger reader callback but we are calling read api to receive data\n");
        printf("   Result - It should trigger read callback and by using read data we can receive same data multiple times.\n");
    }
    scanf("%d", &test);

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
