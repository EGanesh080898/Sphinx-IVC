
#include "IvcReaderWriter.hpp"
#ifdef ACRN_HV_PLATFORM
#include "IvcAcrnRW.hpp"
#endif
#ifdef QNX_HV_PLATFORM
#error "Qnx RW Driver not implemented"
#include "IvcQnxRW.hpp"
#endif

std::mutex cIvcReaderWriter::mlock;

void cIvcReaderWriter::createInstance()
{
#ifdef ACRN_HV_PLATFORM
    instance = std::make_unique<cIvcAcrnRW>();
#endif
#ifdef QNX_HV_PLATFORM
    instance = std::make_unique<cIvcQnxRW>();
#endif
}

cIvcReaderWriter::cIvcReaderWriter()
{
    setIvcLogLevel(IVC_LOG_INFO);
    createInstance();
}

cIvcReaderWriter::~cIvcReaderWriter()
{
}

/**************************************************************************************
 * register all memory related configs.
 *
 * @param in -> clientType: see interface for all possible client types
 * @param in -> memName: name given to shared memory
 * @param in -> maxMemSize: requested shared memory size.
 * @param in -> rCb: reader callback function.
 * @param in -> wCb: writer callback function.
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::setInterfaceConfig(int32_t clientType, std::string memName, ssize_t maxMemSize, readerCallback rCb, writerCallback wCb, void *arg)
{
    std::unique_lock<std::mutex> lock(mlock);

    return instance->setInterfaceConfig(clientType, memName, maxMemSize, rCb, wCb, arg);
}


/**************************************************************************************
 * start reader/writer interface(threads).
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::startInterface ()
{
    std::unique_lock<std::mutex> lock(mlock);

    return instance->startInterface();
}


/**************************************************************************************
 * stop reader/writer interface(threads).
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::stopInterface()
{
    std::unique_lock<std::mutex> lock(mlock);

    return instance->stopInterface();
}


/**************************************************************************************
 * push data to memory and wakeup writer thread.
 *
 * @param in: buffer: buffer from which data needs to be copied
 * @param in: len: size of data to be copied
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::pushData(const void *buffer, const size_t len)
{
    std::unique_lock<std::mutex> lock(mlock);

    return instance->pushData(buffer, len);
}



/**************************************************************************************
 * write data to memory at given offset.
 *
 * @param in: buffer: buffer from which data needs to be copied
 * @param in: len: size of data to be copied
 * @param in: offset: offset at which data needs to be copied
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::writeAtOffset(const void *buffer, const size_t len, const int32_t offset)
{
    return instance->writeAtOffset(buffer, len, offset);
}


/**************************************************************************************
 * pull data from memory and acknowledge reading.
 *
 * @param in: buffer: buffer to which data needs to be copied
 * @param in: len: size of data to be copied
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::pullData(void *buffer, const size_t len)
{
    std::unique_lock<std::mutex> lock(mlock);

    return instance->pullData(buffer, len);
}


/**************************************************************************************
 * read data from memory without acknowledging read.
 *
 * @param in: buffer: buffer to which data needs to be copied
 * @param in: len: size of data to be copied
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::read(void *buffer, const size_t len)
{
    std::unique_lock<std::mutex> lock(mlock);

    return instance->read(buffer, len);
}


/**************************************************************************************
 * read data from memory at given offset.
 *
 * @param in: buffer: buffer to which data needs to be copied
 * @param in: len: size of data to be copied
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::readFromOffset(void *buffer, const size_t len, const int32_t offset)
{
    return instance->readFromOffset(buffer, len, offset);
}

/**************************************************************************************
 * Check for number of clients attached to a shared memory
 *
 * @param out ->  clientsAttached: number of clients attached to shared memory
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::getNumberOfClientsAttached(int32_t &clientsAttached)
{
    std::unique_lock<std::mutex> lock(mlock);
    return instance->getNumberOfClientsAttached(clientsAttached);
}


/**************************************************************************************
 * Get id of client attached to a shared memory
 *
 * @param out ->  clientId: client ID
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcReaderWriter::getClientId(int32_t &clientId)
{
    return instance->getClientId(clientId);
}

