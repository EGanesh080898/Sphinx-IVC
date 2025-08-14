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
#include <sstream>
#include <iomanip>
#include <vector>
#include <dirent.h>
#include <fcntl.h>

#define MAX_PROCFS_PROCESS_PATH   256
#define MAX_THREADS               25
#define MAX_THREADS_SUPPORTED 1024

//#define DEBUG

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
    long long int count;
    long long int size;
}bufInfo;

std::map<std::string, bufInfo> writeInfo;
std::map<std::string, bufInfo> readInfo;
std::vector<std::vector<double>> cpuLoads(MAX_THREADS);
Semaphore smphSignalCallbackToPush[MAX_THREADS_SUPPORTED];
Semaphore smphSignalSync;
ssize_t dataSize = 100;
bool debug = false;
bool monitorThread = false;
uint32_t duration = 0;
uint32_t interval = 1000;
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

void help_function()
{
    std::cout << " '-type | -t'   - this option is to select reader or writer\n";
    std::cout << "                -type w --> to select the writer \n";
    std::cout << "                -type r --> to select the reader \n\n";
    std::cout << " '-count | -c'  - this option is to give the count of readers/writers selected\n";
    std::cout << "                - count 2 --> to select 2 writers \n\n";
    std::cout << " '-topic | -tp' - this option is to give the name of the topic\n";
    std::cout << "                -topic topic_name --> to set topic name as topic_name \n\n";
    std::cout << " '-debug | -d'  - this option is to print data additional debug logs\n";
    std::cout << "                -debug  --> debug logs \n\n";
    std::cout << " '-monitorthreads | -mt' - this option is to enable monitoring at thread level\n\n";
    std::cout << "                -monitorthreads  --> monitor threads \n\n";
    std::cout << " '-bufsize | -bs' - this option is to generate the data if selected as writer or for the knowledge of the reader if reader option is selected\n";
    std::cout << "                -bufsize N --> to create buffer of length N for writer to write \n\n";
    std::cout << " '-sametopic | -st' - this option is to make readers/writers created on the same topic. Without this option, readers/writer on incremental topic i.e. topic0, topic1... will be created\n";
    std::cout << "                -sametopic  --> to make readers/writers created on the same topic \n\n";
    std::cout << " '-interval | -i' - this option is to select intervals at which cpu load will be calculated\n";
    std::cout << "                -interval N -> intervals is in milliseconds\n";
}

////////////////////////////////////////////////////////////////////

void handleExit()
{
    for (auto &it : writeInfo)
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Topic : " << it.first << std::endl;
        std::cout << "cpuload test duration\t: " << duration << " seconds" << std::endl;
        std::cout << "Write Count\t\t\t: " << it.second.count << std::endl;
        std::cout << "Data Size Chunks\t\t: " << (it.second.count ? it.second.size/it.second.count : 0) << " Bytes" << std::endl;
        if(duration)
            std::cout << "Average Throughput\t\t: " << formatBytes((it.second.size) / duration) << " per sec" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
    }

    for (auto &it : readInfo)
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Topic : " << it.first << std::endl;
        if(sametopic == false)
        {
            std::cout << "Read Count\t\t\t: " << it.second.count << std::endl;
            std::cout << "Data Size Chunks\t\t: " << (it.second.count ? it.second.size/it.second.count : 0) << " Bytes" << std::endl;
        }
        else
        {
            std::cout << "Read Count\t\t\t: " << it.second.count / processCount << std::endl;
            std::cout << "Data Size Chunks\t\t: " << (it.second.count ? it.second.size/it.second.count : 0) << " Bytes" << std::endl;
        }
        std::cout << "----------------------------------------------------------" << std::endl;
    }
}


void signal_callback_handler(int signum) {
#ifdef DEBUG
    std::cout << "######################################################################\n"
              << "                         Caught signal\n"
              << signum
              << "######################################################################\n"
              << std::endl;
    std::cout << "Exiting signal Handler.\n";
#endif
    for(int i=0;i<processCount;i++)
        killThreads[i] = true;
    return;
}

int32_t getThreadCount(std::vector<std::string>& tList)
{
    int32_t tc = -2;
    DIR *dirp = NULL;
    struct dirent *dp =NULL;

    dirp = opendir("/proc/self/task/");
    while (dirp) {
        if ((dp = readdir(dirp)) != NULL) {
            if(tc >= 0)
            {
#ifdef DEBUG
                std::cout << "Found: " << dp->d_name << std::endl;
#endif
                tList.push_back(dp->d_name);
            }
            tc++;
        } else {
#ifdef DEBUG
            std::cout << "failed to open" << std::endl;
#endif
            closedir(dirp);
            break;
        }
    }
    return (tc>0) ? tc:0;
}

void monitor_cpu_start(unsigned long& utime_start, unsigned long& stime_start, std::string name) {
    char stat_path[MAX_PROCFS_PROCESS_PATH];

    // Initialize start times
    if(!name.length())
    {
        snprintf(stat_path, sizeof(stat_path), "/proc/self/stat");
    }
    else
    {
        snprintf(stat_path, sizeof(stat_path), "/proc/self/task/%s/stat", name.c_str());
    }
    FILE *stat_file = fopen(stat_path, "r");

    if (stat_file == NULL) {
        fprintf(stderr, "Error: Unable to open stat file for process %s\n", stat_path);
        return;
    }

    fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu",
           &utime_start, &stime_start);

    fclose(stat_file);
}

void getName(char *path, char* name)
{

    int fp = open(path, O_RDONLY);
    size_t len = read(fp, name, 131072);
    name[len-1] = 0;
#ifdef DEBUG
    printf("lala=%ld for %s:%s\n", len, path, name);
#endif
    close(fp);
}

double monitor_cpu_end(unsigned long& utime_start, unsigned long& stime_start, std::string name, int idx) {
    char stat_path[MAX_PROCFS_PROCESS_PATH];
    char name_path[MAX_PROCFS_PROCESS_PATH];
    double cpu_usage = 0;
    unsigned long utime_end=0, stime_end=0;
    char tname[131072] = {0};

    // Initialize end times
    if(!name.length())
    {
        snprintf(stat_path, sizeof(stat_path), "/proc/self/stat");
        snprintf(name_path, sizeof(name_path), "/proc/self/comm");
    }
    else
    {
        snprintf(stat_path, sizeof(stat_path), "/proc/self/task/%s/stat", name.c_str());
        snprintf(name_path, sizeof(name_path), "/proc/self/task/%s/comm", name.c_str());
    }
    FILE *stat_file = fopen(stat_path, "r");

    if (stat_file == NULL) {
        fprintf(stderr, "Error: Unable to open stat file for process %s\n", stat_path);
        return cpu_usage;
    }

    fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu",
           &utime_end, &stime_end);

    fclose(stat_file);

    // Calculate CPU usage percentages
    unsigned long total_time_start = utime_start + stime_start;
    unsigned long total_time_end = utime_end + stime_end;
    unsigned long total_time_diff = total_time_end - total_time_start;

    cpu_usage = ((double)total_time_diff) / sysconf(_SC_CLK_TCK) * (1000/interval) *100;
    cpuLoads[idx].push_back(cpu_usage);
    getName(name_path, tname);
    printf("CPU load : %.2f for %s\n", cpu_usage, tname);

    return cpu_usage;
}

void loadCalculator(int processCount) {
    unsigned long iterations = 0;
    double cpu_usage = 0;
    double loadTotalPct = 0;
    int32_t threadCnt = 0;
    std::vector<std::string> tList;

#ifdef DEBUG
    std::cout << "Starting Load Calculator" << std::endl;
#endif
    if(true == monitorThread)
    {
        pthread_setname_np(pthread_self(), __FUNCTION__);
        sleep(1); // to get proper thread count
        threadCnt = getThreadCount(tList);
        std::cout << "Start monitoring threads: " << threadCnt << std::endl;
    }
    cpuLoads.resize(threadCnt+1);
    unsigned long utime_start[threadCnt+1], stime_start[threadCnt+1];
    for(int i=0; i<processCount; i++)
        smphSignalSync.release();
    while (!killThreads[0]) {
       for(int32_t i=0; i<threadCnt+1;i++)
        {
            utime_start[i] = stime_start[0] = 0;
        }
        monitor_cpu_start(utime_start[0], stime_start[0], "");
        for(int32_t i=0; i<tList.size();i++)
        {
            monitor_cpu_start(utime_start[i+1], stime_start[i+1], tList[i]);
        }
        usleep(interval*1000);
        cpu_usage = monitor_cpu_end(utime_start[0], stime_start[0], "", 0);
        for(int32_t i=0; i<tList.size();i++)
        {
            cpu_usage += monitor_cpu_end(utime_start[i+1], stime_start[i+1], tList[i], i+1);
        }
        if(cpu_usage)
            iterations++;
    }
    int32_t i=0;
    std::cout << "\n-------------------------------------------------------------------------" << std::endl;
    for(auto clv: cpuLoads)
    {
        char name_path[MAX_PROCFS_PROCESS_PATH];
        char tname[131072] = {0};
        loadTotalPct = 0;
        for(auto cl: clv)
        {
            loadTotalPct += cl;
        }
        if(i)
        {
            snprintf(name_path, sizeof(name_path), "/proc/self/task/%s/comm", tList[i-1].c_str());
        }
        else
        {
            snprintf(name_path, sizeof(name_path), "/proc/self/comm");
        }
        getName(name_path, tname);
        double loadAvgPct = iterations ? (loadTotalPct/iterations) : 0;
        std::cout << "Average CPU Load for " << tname << "(" << name_path << ") in Percentage: " << loadAvgPct << std::endl;
        i++;
    }
    std::cout << "-------------------------------------------------------------------------\n" << std::endl;
    for(int i=0; i<processCount; i++)
        smphSignalSync.release();
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
        killThreads[*((int *)(obj.arg))] = true;
        return 0;
    }
    std::string topic = obj.shMemName;
    readInfo[topic].count += 1;
    readInfo[topic].size += obj.bufSize;

    return 0;
}

void launchReader(int rdNo, std::string topic) {
#ifdef DEBUG
    std::cout << "Launching reader with topic : " << topic << ", with ID = " << rdNo << std::endl;
#endif
    readInfo[topic].count = 0;
    readInfo[topic].size = 0;
    std::string threadName = __FUNCTION__ + std::to_string(rdNo);
    pthread_setname_np(pthread_self(), threadName.c_str());

    cIvcReaderWriter *obj = new cIvcReaderWriter();
    if(!obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, topic, dataSize, &readCallBack, NULL, &rdNo))
    {
        smphSignalSync.acquire();
        if(!obj->startInterface())
        {
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            incrementUserCount(topic);
#endif
            while (!killThreads[rdNo]) {
                sleep(1);
            }
            smphSignalSync.acquire();
#ifdef ACRN_ENABLE_IVCHEARTBEAT
            decrementUserCount(topic);
#endif
            obj->stopInterface();
            //handleExit();
        }
        else
        {
            killThreads[rdNo] = true;
            std::cout << "startInterface failed" << std::endl;
        }
    }
    else
    {
        killThreads[rdNo] = true;
        std::cout << "setInterfaceConfig failed" << std::endl;
    }
    delete obj;
#ifdef DEBUG
    std::cout << "Exiting from Reader " + std::to_string(rdNo) << std::endl;
#endif
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
    auto start = std::chrono::high_resolution_clock::now();
    const char *endOfT = "ByeBye!!";
    const uint8_t *message = reinterpret_cast<const uint8_t*>(data);
    std::string threadName = __FUNCTION__ + std::to_string(wrtNo);

    pthread_setname_np(pthread_self(), threadName.c_str());
#ifdef DEBUG
    std::cout << "Launching writer with topic : " << topic << ", with ID = " << wrtNo << std::endl;
#endif
    writeInfo[topic].count = 0;
    writeInfo[topic].size = 0;
    cIvcReaderWriter *obj = new cIvcReaderWriter;
    if(!obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, topic, dataSize, NULL, &writeCallBack, &wrtNo))
    {
        if(obj->startInterface())
        {
            killThreads[wrtNo] = true;
            std::cout << "startInterface failed" << std::endl;
            goto fail;
        }
    }
    else
    {
        killThreads[wrtNo] = true;
        std::cout << "setInterfaceConfig failed" << std::endl;
        goto fail;
    }
    smphSignalSync.acquire();
#ifdef ACRN_ENABLE_IVCHEARTBEAT
    incrementUserCount(topic);
#endif
    start = std::chrono::high_resolution_clock::now();

    while (!killThreads[wrtNo]) {
        obj->pushData(message, dataSize);
        writeInfo[topic].count += 1;
        writeInfo[topic].size += dataSize;
        smphSignalCallbackToPush[wrtNo].acquire();
    }
    smphSignalSync.acquire();
    obj->pushData(endOfT, sizeof(endOfT)); //Send end of transmission packet.
#ifdef ACRN_ENABLE_IVCHEARTBEAT
    decrementUserCount(topic);
#endif
    duration = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::high_resolution_clock::now() - start ).count();
    obj->stopInterface();
fail:
    killThreads[wrtNo] = true;
    delete obj;
#ifdef DEBUG
    std::cout << "\nExiting from Writer-" + std::to_string(wrtNo) << std::endl;
#endif
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
    std::thread loadCalculatorThread;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0) {
            help_function();
            return 0;
        }
        else if (strcmp(argv[i], "-type") == 0 || strcmp(argv[i], "-t") == 0) {
            readerwriter = argv[i+1];
            i++;
        }
        else if (strcmp(argv[i], "-interval") == 0 || strcmp(argv[i], "-i") == 0) {
            interval = std::stoi(argv[i+1]);
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
        }else if (strcmp(argv[i], "-monitorthreads") == 0 || strcmp(argv[i], "-mt") == 0) {
            monitorThread = true;
        }else if (strcmp(argv[i], "-debug") == 0 || strcmp(argv[i], "-d") == 0) {
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

            std::cout << "CPU Load Test Started.. Please Ctrl+C tp stop the test" << std::endl;
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

    loadCalculatorThread = std::thread(loadCalculator, processCount);

    signal(SIGINT, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);

    for (int i = 0; i < processCount; i++) {
        if (processThreads[i].joinable()) {
            processThreads[i].join();
#ifdef DEBUG
            std::cout << "Thread " << i << " joined.\n";
#endif
        }
    }
    if (data) free(data);
    if (loadCalculatorThread.joinable()) {
        loadCalculatorThread.join();
#ifdef DEBUG
        std::cout << "Thread loadCalculator joined.\n";
#endif
    }
    handleExit();
#ifdef DEBUG
    std::cout << "Exiting Main\n";
#endif
    return 0;
}
