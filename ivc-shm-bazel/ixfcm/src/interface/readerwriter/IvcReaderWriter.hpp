#ifndef IVCREADERWRITER_H
#define IVCREADERWRITER_H

#include <memory>
#include <sys/types.h>
#include <mutex>
#include "IvcReaderWriterInterface.hpp"

//namespace lgivc {

class cIvcReaderWriter : public cIvcReaderWriterInterface
{
    std::unique_ptr<cIvcReaderWriterInterface> instance;
    void createInstance();
    static std::mutex mlock;

    public:

        cIvcReaderWriter();
        ~cIvcReaderWriter();

        /*First API called to set clientType(see enum clientType), memName, maxMemSize, reader and writer callbacks*/
        int32_t setInterfaceConfig (int32_t clientType, std::string memName, ssize_t maxMemSize, readerCallback rCb, writerCallback wCb, void *arg);

        /* This API is called when the caller wants to start pushing data or start receiving data to/from shared memory*/
        int32_t startInterface ();

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

        /* This API is called when the caller wants to read data from shared memory from said offset*/
        int32_t readFromOffset(void *buffer, const size_t len, const int32_t offset);

        /* This API is called when the caller wants to know the number of clients attached to a shared memory*/
        int32_t getNumberOfClientsAttached(int32_t &clientsAttached);

        /* This API is called when the caller wants to know the id of the client attached to the shared memory*/
        int32_t getClientId(int32_t &clientId);
};

//}

#endif //IVCREADERWRITER_H

