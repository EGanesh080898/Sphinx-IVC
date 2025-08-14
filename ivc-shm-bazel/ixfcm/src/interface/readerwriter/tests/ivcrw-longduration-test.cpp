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

#define DEBUG

#define DEFAULT_TEST_DURATION 10 //In seconds
#define MAX_PROCFS_PROCESS_PATH   256
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
    long long int count;
    long long int size;
}bufInfo;

std::map<std::string, bufInfo> writeInfo;
std::map<std::string, bufInfo> readInfo;
double loadTotalPct = 0;
Semaphore smphSignalCallbackToPush[MAX_THREADS_SUPPORTED];
Semaphore smphSignalSync;
ssize_t dataSize = 100;
bool debug = false;
bool sametopic = false;
int processCount = 1;
uint32_t duration = DEFAULT_TEST_DURATION;

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
    std::cout << " '-duration | -dt' - this option is to select duration for which test needs to run\n";
    std::cout << "                -duration N<timeunit> -> valid timeunits are s, m, h, d for ex: 11s or 22m or 33h or 44d which corresponds to 11 seconds or 22 minutes or 33 hours or 44 days respectively\n";
}

////////////////////////////////////////////////////////////////////

void handleExit()
{
    //killThreads = true;
    for (auto &it : writeInfo)
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Topic : " << it.first << std::endl;
        std::cout << "Throughput test duration\t: " << duration << " seconds" << std::endl;
        std::cout << "Write Count\t\t\t: " << it.second.count << std::endl;
        std::cout << "Data Size Chunks\t\t: " << (it.second.count ? it.second.size/it.second.count : 0) << " Bytes" << std::endl;
        std::cout << "Average Throughput\t\t: " << formatBytes((it.second.size) / duration ) << " per sec" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
    }
    for (auto &it : readInfo)
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Reader : Topic : " << it.first << std::endl;
        std::cout << "Throughput test duration\t: " << duration << " seconds" << std::endl;
        if(sametopic == false)
        {
            std::cout << "Read Count\t\t\t: " << it.second.count << std::endl;
            std::cout << "Data Size Chunks\t\t: " << (it.second.count ? it.second.size/it.second.count : 0) << " Bytes" << std::endl;
            std::cout << "Average Throughput\t\t: " << formatBytes((it.second.size) / duration ) << " per sec" << std::endl;
        }
        else
        {
            std::cout << "Read Count\t\t\t: " << it.second.count / processCount << std::endl;
            std::cout << "Data Size Chunks\t\t: " << (it.second.count ? it.second.size/it.second.count : 0) << " Bytes" << std::endl;
            std::cout << "Average Throughput\t\t: " << formatBytes((it.second.size / processCount) / duration ) << " per sec" << std::endl;
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
#endif
    //handleExit();
    for(int i=0;i<processCount;i++)
        killThreads[i] = true;
#ifdef DEBUG
    std::cout << "Exiting signal Handler.\n";
#endif
    return;
}

void monitor_mem_start(unsigned long& vsize_start, unsigned long& rss_start) {
    char stat_path[MAX_PROCFS_PROCESS_PATH];

    // Initialize start times
    snprintf(stat_path, sizeof(stat_path), "/proc/self/stat");

    FILE *stat_file = fopen(stat_path, "r");

    if (stat_file == NULL) {
        fprintf(stderr, "Error: Unable to open stat file for process %s\n", stat_path);
        return;
    }

    fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
           &vsize_start, &rss_start);

    fclose(stat_file);
}

void monitor_mem_end(unsigned long& vsize_start, unsigned long& rss_start) {
    char stat_path[MAX_PROCFS_PROCESS_PATH];
    unsigned long vsize_end=0, rss_end=0;

    // Initialize end times
    snprintf(stat_path, sizeof(stat_path), "/proc/self/stat");

    FILE *stat_file = fopen(stat_path, "r");

    if (stat_file == NULL) {
        fprintf(stderr, "Error: Unable to open stat file for process %s\n", stat_path);
        return;
    }

     fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
           &vsize_end, &rss_end);

    fclose(stat_file);

    // Calculate MEM usage
    int64_t total_mem_start = vsize_start + rss_start;
    int64_t total_mem_end = vsize_end + rss_end;
    int64_t total_mem_diff = total_mem_end - total_mem_start;
    if(total_mem_diff > 0)
    {
        printf("MEM usage : vsize_start=%s rss_start=%s vsize_end=%s rss_end=%s total_mem_diff=%s\n", formatBytes(vsize_start).c_str(), formatBytes(rss_start).c_str(), formatBytes(vsize_end).c_str(), formatBytes(rss_end).c_str(), formatBytes(total_mem_diff).c_str());
    }

    return;
}

void monitor_cpu_start(unsigned long& utime_start, unsigned long& stime_start) {
    char stat_path[MAX_PROCFS_PROCESS_PATH];

    // Initialize start times
    snprintf(stat_path, sizeof(stat_path), "/proc/self/stat");

    FILE *stat_file = fopen(stat_path, "r");

    if (stat_file == NULL) {
        fprintf(stderr, "Error: Unable to open stat file for process %s\n", stat_path);
        return;
    }

    fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu",
           &utime_start, &stime_start);

    fclose(stat_file);
}

double monitor_cpu_end(unsigned long& utime_start, unsigned long& stime_start) {
    char stat_path[MAX_PROCFS_PROCESS_PATH];
    double cpu_usage = 0;
    unsigned long utime_end=0, stime_end=0;

    // Initialize end times
    snprintf(stat_path, sizeof(stat_path), "/proc/self/stat");

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
    if(total_time_diff)
    {
        cpu_usage = ((double)total_time_diff) / sysconf(_SC_CLK_TCK) *100;
        loadTotalPct += cpu_usage;
    }
    printf("CPU load : %.2f\n", cpu_usage);
    return cpu_usage;
}

void monitorThread(int processCount) {
    unsigned long iterations = 0;
    double cpu_usage = 0;
    std::vector<std::string> tList;
    unsigned long utime_start, stime_start;
    unsigned long vmem_start, rss_start;
#ifdef DEBUG
    std::cout << "Starting Load Calculator" << std::endl;
#endif
    vmem_start = rss_start = 0;
    for(int i=0; i<processCount; i++)
        smphSignalSync.release();
    while (!killThreads[0]) {
        utime_start = stime_start = 0;
        monitor_cpu_start(utime_start, stime_start);
        monitor_mem_start(vmem_start, rss_start);
        sleep(1);
        cpu_usage = monitor_cpu_end(utime_start, stime_start);
        monitor_mem_end(vmem_start, rss_start);
        if(cpu_usage)
            iterations++;
    }
    std::cout << "\n----------------------------------------------------------" << std::endl;
    std::cout << "Average CPU Load in Percentage: " << (iterations ? (loadTotalPct/iterations) : 0)<< std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

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
    static std::chrono::time_point<std::chrono::high_resolution_clock> start;
    static bool firstTime = true;

    if(true == firstTime)
    {
        start = std::chrono::high_resolution_clock::now();
        std::cout << "First Packet received" << std::endl;
        firstTime = false;
    }
    if(true == debug)
    {
        std::cout << "Read call back triggered with event: " << obj.cbType << " and buffer size: "<< obj.bufSize << std::endl;
    }
    if((obj.bufSize == strlen(endOfT)) && !strncmp((char*)obj.buffer, endOfT, strlen(endOfT)))
    {
        //handleExit();
        std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
        duration = diff.count() + 1;
        std::cout << "Last Packet with duration:" << duration << std::endl;
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
    std::cout << "Launching writer with topic : " << topic << ", with ID = " << wrtNo << std::endl;
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

    while (std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - start ) <= std::chrono::seconds(duration) && !killThreads[wrtNo]) {
        obj->pushData(message, dataSize);
        writeInfo[topic].count += 1;
        writeInfo[topic].size += dataSize;
        smphSignalCallbackToPush[wrtNo].acquire();
    }
    obj->pushData(endOfT, sizeof(endOfT)); //Send end of transmission packet.
    killThreads[wrtNo] = true;
#ifdef ACRN_ENABLE_IVCHEARTBEAT
    decrementUserCount(topic);
#endif
    obj->stopInterface();
fail:
    delete obj;
    //handleExit();
#ifdef DEBUG
    std::cout << "\nExiting from Writer-" + std::to_string(wrtNo) << std::endl;
#endif
    return;
}

bool checkValidDuration(std::string durStr)
{
    bool ret = false;
    ssize_t len = durStr.length();
    if(len)
    {
        const char *dur = durStr.c_str();
        char *num = strdup(dur);
        char *endptr = NULL;
        num[len-1] = 0;
        if(dur[0] == '-')
            return false;
        switch(dur[len-1])
        {
            case 's':
                duration = strtol(num, &endptr, 0);
                std::cout << "seconds duration: " << durStr << ", duration in seconds: " << duration << std::endl;
                ret = true;
                break;

            case 'm':
                duration = strtol(num, &endptr, 0) * 60;
                std::cout << "minutes duration: " << durStr << ", duration in seconds: " << duration << std::endl;
                ret = true;
                break;

            case 'h':
                duration = strtol(num, &endptr, 0) * 60 * 60;
                std::cout << "hours duration: " << durStr << ", duration in seconds: " << duration << std::endl;
                ret = true;
                break;

            case 'd':
                duration = strtol(num, &endptr, 0) * 60 * 60 * 24;
                std::cout << "days duration: " << durStr << ", duration in seconds: " << duration << std::endl;
                ret = true;
                break;

            default:
                std::cout << "Invalid duration: " << durStr << std::endl; 
        }
        free(num);
    }

    return ret;
}

int main(int argc, char* argv[]) {
    std::string readerwriter = "w";
    std::string topic = "topic-";
    std::string durationStr = "11s";
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
        else if (strcmp(argv[i], "-duration") == 0 || strcmp(argv[i], "-dt") == 0) {
            durationStr = argv[i+1];
            if(false == checkValidDuration(durationStr))
            {
                std::cout << "Invalid duration.. Check help!!" << std::endl;
                help_function();
                return -1;
            }
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

            std::cout << "Long duration Test Started.. It will run for " << duration << " secs" << std::endl;
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
    loadCalculatorThread = std::thread(monitorThread, processCount);
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
    std::cout << "Exiting Main\n";
    return 0;
}
