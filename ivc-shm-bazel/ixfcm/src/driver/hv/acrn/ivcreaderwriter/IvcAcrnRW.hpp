#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string.h>
#include <thread>
#include <chrono>
//#include <semaphore>
#include <vector>
#include <unordered_map>

#include "ivcLog.hpp"
#include "IvcReaderWriterInterface.hpp"

#define LGIVC_WRITERTHREAD_READ_WAIT_TIME_MS 1000

enum WRITERTHREAD_STATE {
    LGIVC_WT_WAIT_STATE = 1,
    LGIVC_WT_CHECK_STATE = 2
};

typedef enum {
    LGIVC_RW_INIT_DONE = 1,
    LGIVC_RW_SETCONFIG_DONE = 2,
    LGIVC_RW_STARTINTERFACE_DONE = 3,
}RW_STATE;

class cIvcAcrnRWSemaphore {

    private:
        std::mutex mtx;
        std::condition_variable cv;
        int32_t count;

    public:
        cIvcAcrnRWSemaphore (int32_t count_ = 0) : count(count_) { }

        void release() {
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }

        bool acquire() {
            using namespace std::chrono_literals;
            std::unique_lock<std::mutex> lock(mtx);
            if(cv.wait_for(lock, 200ms, [this] { return count > 0; }))
            {
                if(count > 0) count--;
                return true;
            }
            else
            {
                return false;
            }
        }
};


class cIvcAcrnRW : public cIvcReaderWriterInterface {

    private:
        int32_t clientType;
        RW_STATE rwState;
        std::string memoryName;
        ssize_t memorySize;
        int32_t clientCount;
        void *callerObj;
        std::shared_ptr<void> memManObj;
        volatile bool killWriterThread;
        volatile bool killReaderThread;
        cIvcAcrnRWSemaphore smphSignalPushToThread;

        std::vector<std::thread> clientThreads;
        readerCallback readerCb;
        writerCallback writerCb;
        void readerThread(std::string threadName);
        void writerThread(std::string threadName);

    public:

        cIvcAcrnRW();

        ~cIvcAcrnRW();

        /*First API called to set clientType(see enum clientType), memName, maxMemSize, reader and writer callbacks*/
        int32_t setInterfaceConfig (int32_t clientType, std::string memName, ssize_t maxMemSize, readerCallback rCb, writerCallback wCb, void* arg);

        /* This API is called when the caller wants to start pushing data or start receiving data to/from shared memory*/
        int32_t startInterface();

        /* This API is called when the caller is no longer interested in pushing data or receiving data to/from shared memory*/
        int32_t stopInterface();

        /* This API is called when the caller wants to write data to shared memory*/
        int32_t pushData(const void *buffer, const size_t len);

        /* This API is called when the caller wants to write data to shared memory at said offset*/
        int32_t writeAtOffset(const void *buffer, const size_t len, const int32_t offset);

        /* This API is called when the caller wants to read data from shared memory and acknowledge the read*/
        int32_t pullData(void *buffer, const size_t len);

        /* This API is called when the caller wants to read data from shared memory without acknowledging the read(snooping data)*/
        int32_t read(void *buffer, const size_t len);

        /* This API is called when the caller wants to read data from shared memory at said offset*/
        int32_t readFromOffset(void *buffer, const size_t len, const int32_t offset);

        /* This API is called when the caller wants to know the number of clients attached to a shared memory*/
        int32_t getNumberOfClientsAttached(int32_t &clientsAttached);

        /* This API is called when the caller wants to know the id of the client attached to the shared memory*/
        int32_t getClientId(int32_t &clientId);
};
