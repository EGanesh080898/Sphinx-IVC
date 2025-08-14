#ifndef IVCREADERWRITERINTERFACE_H
#define IVCREADERWRITERINTERFACE_H

#include <sys/types.h>
#include <string>

typedef enum
{
    IVC_READERWRITER_DATA_CALLBACK, //Received callback along with data when registered as LGIVC_WITH_DATACALLBACK
    IVC_READERWRITER_EVENT_CALLBACK //Received callback along with data when registered as LGIVC_WITH_EVENTCALLBACK
}readerCbStatus;

typedef struct
{
    std::string     shMemName;
    readerCbStatus  cbType;
    void            *buffer;
    size_t          bufSize;
    void            *arg;
}readerObj;

typedef enum
{
    IVC_READERWRITER_READ_COMPLETED, //Indicates all readers have read the data
    IVC_READERWRITER_READ_TIMEOUT  //Indicates 1 or more readers haven't read the data
}writerCbStatus;

typedef struct
{
    std::string     shMemName;
    writerCbStatus  cbType;
    void            *arg;
}writerObj;


typedef  bool (*readerCallback)(readerObj &obj);
typedef bool (*writerCallback)(writerObj &obj);

enum clientType {
    LGIVC_SYNC_READER                        = 1<<1, //Reader without callback
    LGIVC_ASYNC_READER                       = 1<<2, //Reader with callback - select type of callback i.e. data or event
    LGIVC_SYNC_WRITER                        = 1<<3, //Writer without callback
    LGIVC_ASYNC_WRITER                       = 1<<4, //Writer with callback
    LGIVC_WITH_DATACALLBACK                  = 1<<5, //Reader with data callback - Only for reader
    LGIVC_WITH_EVENTCALLBACK                 = 1<<6  //Reader with event callback
};

class cIvcReaderWriterInterface {

    public:

        virtual ~cIvcReaderWriterInterface() {};

        /*First API called to set clientType(see enum clientType), memName, maxMemSize, reader and writer callbacks*/
        virtual int32_t setInterfaceConfig (int32_t clientType, std::string memName, ssize_t maxMemSize, readerCallback rCb, writerCallback wCb, void *arg) = 0;

        /* This API is called when the caller wants to start pushing data or start receiving data to/from shared memory*/
        virtual int32_t startInterface () = 0;

        /* This API is called when the caller is no longer interested in pushing data or receiving data to/from shared memory*/
        virtual int32_t stopInterface() = 0;

        /* This API is called when the caller wants to write data to shared memory*/
        virtual int32_t pushData(const void *buffer, const size_t len) = 0;

        /* This API is called when the caller wants to write data to shared memory at said offset*/
        virtual int32_t writeAtOffset(const void *buffer, const size_t len, const int32_t offset) = 0;

        /* This API is called when the caller wants to read data from shared memory and acknowledge the read*/
        virtual int32_t pullData(void *buffer, const size_t len) = 0;

        /* This API is called when the caller wants to read data from shared memory without acknowledging the read(snooping data)*/
        virtual int32_t read(void *buffer, const size_t len) = 0;

        /* This API is called when the caller wants to read data from shared memory from said offset*/
        virtual int32_t readFromOffset(void *buffer, const size_t len, const int32_t offset) = 0;

        /* This API is called when the caller wants to know the number of clients attached to a shared memory*/
        virtual int32_t getNumberOfClientsAttached(int32_t &clientsAttached) = 0;

        /* This API is called when the caller wants to know the id of the client attached to the shared memory*/
        virtual int32_t getClientId(int32_t &clientId) = 0;
};

#endif //IVCREADERWRITERINTERFACE_H
