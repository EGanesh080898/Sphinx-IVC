#ifndef IVC_LOGGER__H
#define IVC_LOGGER__H

#include <iostream>
#include <stdarg.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <fstream>

#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#define IVC_LOG_FILE "/tmp/ivc_log.txt"
#define IVC_CONF_FILE "/tmp/ivc_config.json"

#define IVC_LOG_ENABLE

typedef enum {
    IVC_LOG_NONE,
    IVC_LOG_ENTEXT,
    IVC_LOG_DEBUG,
    IVC_LOG_INFO,
    IVC_LOG_WARNING,
    IVC_LOG_ERROR,
    IVC_LOG_CRITICAL,
    IVC_LOG_FATAL
} IvcLogLevel;

#ifdef IVC_LOG_ENABLE
#define IVC_LOG_ENTEXT(format, ...) Ivc_Log(IVC_LOG_ENTEXT, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_DEBUG(format, ...) Ivc_Log(IVC_LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_INFO(format, ...) Ivc_Log(IVC_LOG_INFO, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_WARNING(format, ...) Ivc_Log(IVC_LOG_WARNING, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_ERROR(format, ...) Ivc_Log(IVC_LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_CRITICAL(format, ...) Ivc_Log(IVC_LOG_CRITICAL, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_FATAL(format, ...) Ivc_Log(IVC_LOG_FATAL, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#else
#define IVC_LOG_ENTEXT(format, ...)
#define IVC_LOG_DEBUG(format, ...)
#define IVC_LOG_INFO(format, ...)
#define IVC_LOG_WARNING(format, ...)
#define IVC_LOG_ERROR(format, ...)
#define IVC_LOG_CRITICAL(format, ...)
#define IVC_LOG_FATAL(format, ...)
#endif

inline void setLogEnv(std::uint8_t level)
{
    char buffer[10] = {0};
    sprintf(buffer, "%d", level);
    setenv("IVC_LOG_LEVEL", buffer, 1);
}

inline std::uint8_t getLogEnv()
{
    std::uint8_t level = 0;
    char* buffer = getenv("IVC_LOG_LEVEL");
    if (buffer){
        level = (std::uint8_t)atoi(buffer);
    }
    return level ? level : 0xFF << IVC_LOG_INFO;
}

static std::uint8_t gLogLevel = std::uint8_t(0xFF << IVC_LOG_ERROR);
#define MAX_QUEUE_SIZE 1000000


inline std::string  getTimeStampNow(std::chrono::system_clock::time_point& now)
{
    std::string currentTime;
    auto time = std::chrono::system_clock::to_time_t(now);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() % 1000000000;

    // Convert the time to a string
    std::stringstream oss;
    oss << '[' << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(9) << nanoseconds << "] ";

    // Store the current time with nanosecond details into a string
    currentTime = oss.str();
    return currentTime;
}

class IvcLogger {
private:
    IvcLogger();
    volatile bool mQuit;
    std::thread mDispatcherThread;
    std::thread mConfMonitorThread;
    std::condition_variable mCond;
    std::mutex mQMutex;
    std::mutex fileMutex;
    std::mutex writeConfMutex;
    std::mutex readConfMutex;
    std::deque<std::pair<std::chrono::system_clock::time_point, std::string>> mQueue1;
    std::deque<std::pair<std::chrono::system_clock::time_point, std::string>> mQueue2;
    std::string logFilePath;
    int logLevel;
    int maxLogFileSizeMB;
    bool overwrite;

public:
    static IvcLogger& getInstance();
    ~IvcLogger();
    void init();
    void dispatcher(std::string tName);
    void monitor(std::string tName);
    void readConfig();
    void writeConfig();
    void enqueue(std::string&);
    void writeToFile(std::pair<std::chrono::system_clock::time_point, std::string>);
    void setLogLevel(IvcLogLevel);
};

inline IvcLogger& IvcLogger::getInstance() {
    static IvcLogger obj;
    return obj;
}

inline IvcLogger::IvcLogger():mQuit(true), logFilePath(IVC_LOG_FILE)
, logLevel(IVC_LOG_DEBUG), maxLogFileSizeMB(10), overwrite(true) {
}

inline IvcLogger::~IvcLogger(){
    mQuit = true;
    mCond.notify_all();
    if (mDispatcherThread.joinable())
        mDispatcherThread.join();
    if (mConfMonitorThread.joinable())
        mConfMonitorThread.join();
}

inline void IvcLogger::init() {
    if (!mQuit) return;
    mQuit = false;
    mQueue1.clear();
    mQueue2.clear();
    mDispatcherThread = std::thread(&IvcLogger::dispatcher, this, "dispatcher");
    mConfMonitorThread = std::thread(&IvcLogger::monitor, this, "monitor");
    mConfMonitorThread.detach();
    mDispatcherThread.detach();
}

inline void IvcLogger::dispatcher(std::string tName){
    std::unique_lock<std::mutex> lock(mQMutex);
    pthread_setname_np(pthread_self(), tName.c_str());
    do {
        mCond.wait(lock, [this] {
            return (mQuit || !mQueue1.empty() || !mQueue2.empty());
        });
        if (!mQuit && !mQueue1.empty()) {
            std::pair<std::chrono::system_clock::time_point, std::string> msg = std::move(mQueue1.front());
            mQueue1.pop_front();
            lock.unlock();
            writeToFile(msg);
            lock.lock();
        }
        if (!mQuit && !mQueue2.empty()) {
            std::pair<std::chrono::system_clock::time_point, std::string> msg = std::move(mQueue2.front());
            mQueue2.pop_front();
            lock.unlock();
            writeToFile(msg);
            lock.lock();
        }
    } while(!mQuit);
}

inline void IvcLogger::monitor(std::string tName){
    struct stat prevStat;
    pthread_setname_np(pthread_self(), tName.c_str());
    if (lstat(IVC_CONF_FILE, &prevStat) != -1) {
        do  {
            struct stat currentStat;
            if (lstat(IVC_CONF_FILE, &currentStat) != -1) {
                if (currentStat.st_mtime != prevStat.st_mtime) {
                    // File modification time has changed
                    prevStat = currentStat;
                    IvcLogger::getInstance().readConfig();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (!mQuit);
    }
}

inline void IvcLogger::readConfig() {
    std::lock_guard<std::mutex> lock(readConfMutex);
    {
    std::ifstream jsonFile(IVC_CONF_FILE);
    int retryCounter = 5;
    while (!jsonFile.is_open()){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        jsonFile.open(IVC_CONF_FILE);
        retryCounter--;
        if (retryCounter<0) break;
    }
    if (!jsonFile.is_open()){
        std::cout << "Unable to Open " << IVC_CONF_FILE << " file" << std::endl;
        return;
    }
    // Parse the JSON content
    try {
        // Parse the JSON content with strict mode disabled
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string parseErrors;

        builder["strictRoot"] = false; // Disable strict mode

        if (!Json::parseFromStream(builder, jsonFile, &root, &parseErrors)) {
            std::cerr << "Failed to parse JSON: " << parseErrors << std::endl;
            return;
        }
        // Access and process the JSON data
        // Example: Print the value of a specific key
        logFilePath = root["logFilePath"].asString();
        logLevel = root["logLevel"].asInt();
        if (logLevel == IVC_LOG_NONE)
            setLogEnv((std::uint8_t)logLevel);
        else if (logLevel>=IVC_LOG_DEBUG && logLevel<=IVC_LOG_FATAL)
            setLogEnv(std::uint8_t(0xFF << logLevel));
        maxLogFileSizeMB = root["maxLogFileSizeMB"].asInt();
        overwrite = root["overwrite"].asBool();
    }
    catch (...) {
        std::cerr << "Failed to parse JSON" << std::endl;
    }
    jsonFile.close();
    }
}

inline void IvcLogger::writeConfig() {
    Json::Value root;
    root["maxLogFileSizeMB"] = maxLogFileSizeMB;
    root["logFilePath"] = logFilePath;
    root["//Log Levels"] = "0: None, 1: EnterExit, 2: Debug, 3: Info, 4: Warning, 5: Error, 6: Critical, 7: Fatal";
    root["logLevel"] = logLevel;
    root["overwrite"] = overwrite;

    // Convert the JSON object to a string with proper spacing
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "    ";  // Set the indentation level (four spaces)

    std::string jsonString = Json::writeString(builder, root);

    std::lock_guard<std::mutex> lock(writeConfMutex);
    {
    // Write the JSON string to a file
    std::ofstream outputFile(IVC_CONF_FILE);
    if (outputFile.is_open()) {
        outputFile << jsonString;
        outputFile.close();
    }
    }
}

inline void IvcLogger::writeToFile(std::pair<std::chrono::system_clock::time_point, std::string> req) {
    std::lock_guard<std::mutex> lock(fileMutex);
    {

    std::chrono::system_clock::time_point& now = req.first;
    std::string timeStamp = getTimeStampNow(now);
    std::string &msg = req.second;
#if 1
    std::cout << timeStamp << msg << std::endl;
#else
    std::fstream file(logFilePath.c_str(), std::ios::app);
    if (!file) {
        // File does not exist, create a new file
        file.close();
        file.open(logFilePath.c_str(), std::ios::app);
    }
    if (file.is_open()) {
        std::streampos fileSize = file.tellg();
        std::uintmax_t bytes = static_cast<std::uintmax_t>(fileSize);
        double mbSize =  static_cast<double>(bytes) / 1048576.0;

        if ((maxLogFileSizeMB != -1) && ((int)mbSize >= maxLogFileSizeMB))
        {
            if (overwrite) {
                file << timeStamp << msg << std::endl << std::flush;
            }
            else {
                // Ignore writing anymore logs since file size limit is reached
                // And overwriting is not allowed
            }
        } else {
            file << timeStamp << msg << std::endl;
        }
        file.close();
    }
#endif
    }
}

inline void IvcLogger::setLogLevel(IvcLogLevel level) {
    if (logLevel<IVC_LOG_NONE && logLevel>IVC_LOG_FATAL) return;
    if (level == logLevel) return;
    logLevel = level;
    writeConfig();
    init();
}

inline void IvcLogger::enqueue(std::string& msg){
    std::lock_guard<std::mutex> lock(mQMutex);
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        if (mQuit) {
            IvcLogger::getInstance().writeToFile({now, msg});
            return;
        }

        if (mQueue1.size() <= MAX_QUEUE_SIZE)
            mQueue1.emplace_back(std::make_pair(now, msg));
        else
            mQueue1.emplace_back(std::make_pair(now, msg));
        mCond.notify_all();
    }
}

inline void setIvcLogLevel(IvcLogLevel level){
    std::uint8_t eLogLevel = 0;
    switch(level){
        case IVC_LOG_NONE:
            eLogLevel = 0;
            setLogEnv(eLogLevel);
            break;
        case IVC_LOG_ENTEXT:
            eLogLevel = std::uint8_t(0xFF << 1);
            setLogEnv(eLogLevel);
            break;
        case IVC_LOG_DEBUG:
            eLogLevel = std::uint8_t(0xFF << 2);
            setLogEnv(eLogLevel);
            break;
        case IVC_LOG_INFO:
            eLogLevel = std::uint8_t(0xFF << 3);
            setLogEnv(eLogLevel);
            break;
        case IVC_LOG_WARNING:
            eLogLevel = std::uint8_t(0xFF << 4);
            setLogEnv(eLogLevel);
            break;
        case IVC_LOG_ERROR:
            eLogLevel = std::uint8_t(0xFF << 5);
            setLogEnv(eLogLevel);
            break;
        case IVC_LOG_CRITICAL:
            eLogLevel = std::uint8_t(0xFF << 6);
            setLogEnv(eLogLevel);
            break;
        case IVC_LOG_FATAL:
            eLogLevel = std::uint8_t(0xFF << 7);
            setLogEnv(eLogLevel);
            break;
        default:
            break;
    }
    
    IvcLogger::getInstance().setLogLevel(level);
}

inline const char* LogStr[] = {"[ENTEXT] ", "[DEBUG] ", "[INFO] ", "[WARNING] ", "[ERROR] ", "[CRITICAL] ", "[FATAL] "};

inline void Ivc_Log(IvcLogLevel level, const char *file, const char *function, int line, const char* format, ...) {
    std::uint8_t eLogLevel = getLogEnv();
    if (level <= 0 || level > IVC_LOG_FATAL || eLogLevel == 0 || (eLogLevel & (1 << level)) == 0) {
        return;
    }
    // Initialize variable argument list
    va_list args;
    va_start(args, format);

    // Format the log message using a std::ostringstream
    char buffer[512] = {0};
    int len = 0;
    sprintf(buffer, "%s:%d:%s:  ", file, line, function);
    len = strlen(buffer);
    vsnprintf(buffer+len, sizeof(buffer)-len, format, args);
    va_end(args);

    std::ostringstream oss;
    oss << LogStr[level-1] << buffer;
    std::string logMessage = oss.str();

    // Clean up the variable argument list
    IvcLogger::getInstance().enqueue(logMessage);
}

#endif
