#include "IvcAcrnRW.hpp"
#include "MemoryManager.hpp"
#include <unistd.h>

/**************************************************************************************
 * Constructor
 * create Memory Manager Object and semaphore used by writer thread
 **************************************************************************************/
cIvcAcrnRW::cIvcAcrnRW()
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    memManObj = std::make_shared<cMemoryManager>();
    clientCount = 0;
    rwState = LGIVC_RW_INIT_DONE;
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
}

/**************************************************************************************
 * Destructor
 **************************************************************************************/
cIvcAcrnRW::~cIvcAcrnRW()
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
}

/**************************************************************************************
 * readerThread
 * This threads waits for interrupt from other clients attached to shared memory.
 * On receiving interrupt, depending of client type, it either reads the data and passed it to the client(LGIVC_WITH_DATACALLBACK)
 * or just notifies the client that event was received
 **************************************************************************************/
void cIvcAcrnRW::readerThread(std::string threadName)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    //IVC_LOG_INFO("cIvcAcrnRW::readerThread started.");
    IVC_LOG_INFO("cIvcAcrnRW::readerThread started - %s", threadName.c_str());
    ssize_t shMemSize = 0;
    ssize_t bytesAvailable = 0;
    int32_t status;
    int64_t count = 0;
    readerObj obj;
    void *bufferData = NULL;

    pthread_setname_np(pthread_self(), threadName.c_str());

    if(reinterpret_cast<cMemoryManager*>(memManObj.get())->get_shMemSize(shMemSize) || shMemSize <=0)
    {
        IVC_LOG_ERROR("No shared memory allocated. get_shMemSize returned size=%ld.\n", shMemSize);
        IVC_LOG_ERROR("cIvcAcrnRW::readerThread exited.");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return;
    }
    bufferData = malloc(shMemSize + 1);
    if (bufferData == NULL) {
        IVC_LOG_CRITICAL("Memory Error while creating bufferData.\n");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return;
    }
    memset(bufferData, 0x0000, shMemSize + 1);
    reinterpret_cast<cMemoryManager*>(memManObj.get())->getNumberOfClientsAttached(clientCount);
    IVC_LOG_INFO("cIvcAcrnRW::readerThread clients connected=%d\n", clientCount);

    while (!killReaderThread) {
        status = reinterpret_cast<cMemoryManager*>(memManObj.get())->wait_for_data();
        if (status) {
            reinterpret_cast<cMemoryManager*>(memManObj.get())->getNumberOfClientsAttached(clientCount);
            continue;
        }

        if(LGIVC_WITH_DATACALLBACK & clientType)
        {
            bytesAvailable = 0;
            status = reinterpret_cast<cMemoryManager*>(memManObj.get())->get_bytes_available(bytesAvailable);
            if (status || bytesAvailable <= 0) {
                if (count % 10 == 0)
                {
                    IVC_LOG_ERROR("get_bytes_available failed no data = %ld", bytesAvailable);
                }
                count += 1;
            }
            else
            {
                IVC_LOG_DEBUG("get_bytes_available success with %d to read in memory.\n", bytesAvailable);
                status = reinterpret_cast<cMemoryManager*>(memManObj.get())->copy_from_memory(bufferData, bytesAvailable);
                if (status)
                {
                    IVC_LOG_ERROR("Error occured in copy_from_memory.\n");
                }
                else {
                    reinterpret_cast<cMemoryManager*>(memManObj.get())->acknowledge_read();
                    reinterpret_cast<cMemoryManager*>(memManObj.get())->get_shMemName(obj.shMemName);
                    obj.cbType = IVC_READERWRITER_DATA_CALLBACK;
                    obj.buffer = bufferData;
                    obj.bufSize = bytesAvailable;
                    obj.arg = callerObj;
                    readerCb(obj);
                }
            }
        }
        else if(LGIVC_WITH_EVENTCALLBACK & clientType)
        {
            reinterpret_cast<cMemoryManager*>(memManObj.get())->get_shMemName(obj.shMemName);
            obj.cbType = IVC_READERWRITER_EVENT_CALLBACK;
            obj.arg = callerObj;
            readerCb(obj);
            count += 1;
        }
    }
    free(bufferData);
    IVC_LOG_INFO("cIvcAcrnRW::readerThread spurious interrupt count = %ld", count);
    IVC_LOG_INFO("cIvcAcrnRW::readerThread completed.");
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
}

/**************************************************************************************
 * writerThread
 * This thread waits for event from write function to wakeup
 * When data is written, this thread checks if all the readers have read the data.
 * If all readers have read data with 1 milliseconds, it notifies IVC_READERWRITER_READ_COMPLETED, else IVC_READERWRITER_READ_TIMEOUT.
 **************************************************************************************/
void cIvcAcrnRW::writerThread(std::string threadName)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_INFO("cIvcAcrnRW::writerThread started - %s", threadName.c_str());
    int state = LGIVC_WT_WAIT_STATE;
    std::chrono::milliseconds waitTime(LGIVC_WRITERTHREAD_READ_WAIT_TIME_MS);
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start;
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    writerObj obj;
    bool semSig = false;
    ssize_t shMemSize = 0;

    pthread_setname_np(pthread_self(), threadName.c_str());

    if(reinterpret_cast<cMemoryManager*>(memManObj.get())->get_shMemSize(shMemSize) || shMemSize <=0)
    {
        IVC_LOG_ERROR("No shared memory allocated. get_shMemSize returned size=%ld.\n", shMemSize);
        IVC_LOG_ERROR("cIvcAcrnRW::writerThread exited.");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return;
    }
    reinterpret_cast<cMemoryManager*>(memManObj.get())->getNumberOfClientsAttached(clientCount);
    IVC_LOG_INFO("cIvcAcrnRW::writerThread clients connected=%d\n", clientCount);

    while (!killWriterThread) {
        switch (state) {
            case LGIVC_WT_WAIT_STATE:
            {
                semSig = smphSignalPushToThread.acquire();
                if(true == killWriterThread)
                    break;
                if(true == semSig)
                {
                    state = LGIVC_WT_CHECK_STATE;
                    start = std::chrono::high_resolution_clock::now();
                }
                else
                {
                    reinterpret_cast<cMemoryManager*>(memManObj.get())->getNumberOfClientsAttached(clientCount);
                }
                break;
            }
            case LGIVC_WT_CHECK_STATE:
            {
                int32_t status = 0;
                status = reinterpret_cast<cMemoryManager*>(memManObj.get())->wait_for_data();

                if (!status) {
                    if((!reinterpret_cast<cMemoryManager*>(memManObj.get())->get_read_status(status)) && (status == 1))
                    {
                        IVC_LOG_DEBUG("Data read! Callback from writer thread.\n");
                        //reinterpret_cast<cMemoryManager*>(memManObj.get())->clearMemory();
                        reinterpret_cast<cMemoryManager*>(memManObj.get())->get_shMemName(obj.shMemName);
                        obj.cbType = IVC_READERWRITER_READ_COMPLETED;
                        obj.arg = callerObj;
                        writerCb(obj);
                        state = LGIVC_WT_WAIT_STATE;
                        break;
                    }
                }
                else
                {
                    end = std::chrono::high_resolution_clock::now();
                    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    reinterpret_cast<cMemoryManager*>(memManObj.get())->getNumberOfClientsAttached(clientCount);
                    if (elapsed >= waitTime) {
                        IVC_LOG_WARNING("waitTime out from writer thread.\n");
                        //reinterpret_cast<cMemoryManager*>(memManObj.get())->clearMemory();
                        reinterpret_cast<cMemoryManager*>(memManObj.get())->get_shMemName(obj.shMemName);
                        obj.cbType = IVC_READERWRITER_READ_TIMEOUT;
                        obj.arg = callerObj;
                        writerCb(obj);
                        state = LGIVC_WT_WAIT_STATE;
                    }
                }
                break;
            }
            default:
            {
                IVC_LOG_CRITICAL("State Error in writer thread.");
                break;
            }
        }
    }

    IVC_LOG_INFO("cIvcAcrnRW::writerThread completed.");
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
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
int32_t cIvcAcrnRW::setInterfaceConfig (int clientType, std::string memName, ssize_t maxMemSize,
                                        readerCallback rCb, writerCallback wCb, void* arg) {
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(rwState != LGIVC_RW_INIT_DONE)
    {
        IVC_LOG_ERROR("cIvcAcrnRW::setInterfaceConfig already set up\n");

        return 1;
    }

    this->writerCb = nullptr;
    this->readerCb = nullptr;
    callerObj = arg;
    if(clientType == LGIVC_WITH_DATACALLBACK)
    {
        IVC_LOG_ERROR("In cIvcAcrnRW::setInterfaceConfig, invalid clientType=%d\n", clientType);
        return 1;
    }
    if(clientType == LGIVC_WITH_EVENTCALLBACK)
    {
        IVC_LOG_ERROR("In cIvcAcrnRW::setInterfaceConfig, invalid clientType=%d\n", clientType);
        return 1;
    }
    if(clientType & LGIVC_ASYNC_WRITER)
    {
        if((clientType & LGIVC_WITH_DATACALLBACK) && !(clientType & LGIVC_ASYNC_READER))
        {
            IVC_LOG_ERROR("In cIvcAcrnRW::setInterfaceConfig, LGIVC_ASYNC_WRITER with invalid clientType=%d\n", clientType);
            return 1;
        }
        if(nullptr != wCb)
        {
            this->writerCb = wCb;
        }
        else
        {
            IVC_LOG_ERROR("In cIvcAcrnRW::setInterfaceConfig, LGIVC_ASYNC_WRITER with clientType=%d and callback=%p\n", clientType, wCb);
            return 1;
        }
    }
    if(clientType & LGIVC_ASYNC_READER)
    {
        if(((LGIVC_WITH_EVENTCALLBACK & clientType) || (LGIVC_WITH_DATACALLBACK & clientType)) && (nullptr != rCb))
        {
            this->readerCb = rCb;
        }
        else
        {
            IVC_LOG_ERROR("In cIvcAcrnRW::setInterfaceConfig, LGIVC_ASYNC_READER with clientType=%d and callback=%p\n", clientType, rCb);
            return 1;
        }
    }

    if(clientType & LGIVC_SYNC_READER)
    {
        if((clientType & LGIVC_WITH_DATACALLBACK) || (clientType & LGIVC_WITH_EVENTCALLBACK))
        {
            IVC_LOG_ERROR("In cIvcAcrnRW::setInterfaceConfig, LGIVC_SYNC_READER with invalid clientType=%d\n", clientType);
            return 1;
        }
        if(nullptr != rCb)
        {
            IVC_LOG_WARNING("In cIvcAcrnRW::setInterfaceConfig, Callback passed to sync reader ignored\n.");
        }
    }

    if(clientType & LGIVC_SYNC_WRITER)
    {
        if(((clientType & LGIVC_WITH_DATACALLBACK) || (clientType & LGIVC_WITH_EVENTCALLBACK)) && !(clientType & LGIVC_ASYNC_READER))
        {
            IVC_LOG_ERROR("In cIvcAcrnRW::setInterfaceConfig, LGIVC_SYNC_WRITER with invalid clientType=%d\n", clientType);
            return 1;
        }
        if(nullptr != wCb)
        {
            IVC_LOG_WARNING("In cIvcAcrnRW::setInterfaceConfig: Callback passed to sync writer ignored\n.");
        }
    }
    this->clientType = clientType;
    if(memName.length())
    {
        IVC_LOG_DEBUG("cIvcAcrnRW::setInterfaceConfig memory name=%s.\n", memName.c_str());
        memoryName = memName;
    }
    else
    {
        IVC_LOG_ERROR("cIvcAcrnRW::setInterfaceConfig no memory name given\n");
        return 1;
    }
    if(maxMemSize>0)
    {
        IVC_LOG_DEBUG("cIvcAcrnRW::setInterfaceConfig memory size=%d.\n", maxMemSize);
        memorySize = maxMemSize;
    }
    else
    {
        IVC_LOG_ERROR("cIvcAcrnRW::setInterfaceConfig can allocate shared memory with size=%ld\n", maxMemSize);
        return 1;
    }
    this->killWriterThread = true;
    this->killReaderThread = true;
    IVC_LOG_DEBUG("cIvcAcrnRW::setInterfaceConfig completed.");

    rwState = LGIVC_RW_SETCONFIG_DONE;
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}

/**************************************************************************************
 * start reader and writer thread.
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcAcrnRW::startInterface ()
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(rwState != LGIVC_RW_SETCONFIG_DONE)
    {
        IVC_LOG_ERROR("cIvcAcrnRW::startInterface already set up or cIvcAcrnRW::setInterfaceConfig was not set up\n");

        return 1;
    }

    cMemoryManager* memMgr = reinterpret_cast<cMemoryManager*>(memManObj.get());
    if(memMgr->getSharedMemory(memoryName, memorySize))
    {
        IVC_LOG_DEBUG("cIvcAcrnRW::startInterface getSharedMemory failed\n.");
        return 1;
    }

    if (readerCb != nullptr) {
        killReaderThread = false;
        std::thread readThread(&cIvcAcrnRW::readerThread, this, memoryName + "_rt");
        clientThreads.push_back(move(readThread));
    }

    if (writerCb != nullptr) {
        killWriterThread = false;
        std::thread writeThread(&cIvcAcrnRW::writerThread, this, memoryName + "_wt");
        clientThreads.push_back(move(writeThread));
    }

    rwState = LGIVC_RW_STARTINTERFACE_DONE;
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}

/**************************************************************************************
 * stop reader and writer thread.
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcAcrnRW::stopInterface()
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(rwState != LGIVC_RW_STARTINTERFACE_DONE)
    {
        rwState = LGIVC_RW_INIT_DONE;
        IVC_LOG_ERROR("cIvcAcrnRW::startInterface was not set up\n");

        return 1;
    }

    IVC_LOG_DEBUG("Releasing the locks and killing the threads.");

    killWriterThread = true;
    killReaderThread = true;
    smphSignalPushToThread.release();

    for (auto &thread:clientThreads)
    {
        if (thread.joinable())
            thread.join();
    }
    IVC_LOG_DEBUG("Released and Killed the threads successfully.");

    if(reinterpret_cast<cMemoryManager*>(memManObj.get())->releaseSharedMemory())
    {
        IVC_LOG_DEBUG("cIvcAcrnRW::setInterfaceConfig getSharedMemory failed\n.");
        return 1;
    }

    rwState = LGIVC_RW_INIT_DONE;
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
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
int32_t cIvcAcrnRW::pushData (const void *buffer, const size_t len)
{
    int32_t ret = -1;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(rwState != LGIVC_RW_STARTINTERFACE_DONE)
    {
        IVC_LOG_ERROR("cIvcAcrnRW::startInterface was not set up\n");

        return 1;
    }

    //This API can be called by only sync and async writers. Others are forbidden
    if (!(clientType & LGIVC_ASYNC_WRITER) && !(clientType & LGIVC_SYNC_WRITER)){
        IVC_LOG_ERROR("invalid clientType=%d.\n", clientType);
        return 1;
    }

    if (!buffer || !len) {
        IVC_LOG_ERROR("Invalid args buffer=%p len=%ld!!\n", buffer, len);
        return 1;
    }

    ret = reinterpret_cast<cMemoryManager*>(memManObj.get())->copy_to_memory(buffer, len);
    if(!ret)
    {
        ret = reinterpret_cast<cMemoryManager*>(memManObj.get())->notifyClients(); //TODO: Can we pass client list?
        if (!ret) {
            smphSignalPushToThread.release();
            IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
            return 0;
        }
    }
    IVC_LOG_ERROR("%s failed\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
    return 1;
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
int32_t cIvcAcrnRW::writeAtOffset(const void *buffer, const size_t len, int32_t offset)
{
    int32_t ret = -1;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    //This API can be called by only sync and async writers. Others are forbidden
    if (!(clientType & LGIVC_ASYNC_WRITER) && !(clientType & LGIVC_SYNC_WRITER)){
        IVC_LOG_ERROR("invalid clientType=%d.\n", clientType);
        return 1;
    }

    if (!buffer || !len) {
        IVC_LOG_ERROR("Invalid args buffer=%p len=%ld!!\n", buffer, len);
        return 1;
    }

    if(!reinterpret_cast<cMemoryManager*>(memManObj.get())->copy_to_memory(buffer, len, offset))
    {
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 0;
    }

    IVC_LOG_ERROR("%s failed\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
    return 1;
}


/**************************************************************************************
 * read data from given offset.
 *
 * @param in: buffer: buffer from which data needs to be copied
 * @param in: len: size of data to be copied
 * @param in: offset: offset at which data needs to be copied
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcAcrnRW::readFromOffset(void *buffer, const size_t len, int32_t offset)
{
    int32_t ret = -1;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    //This API can be called by only sync readers. Others are forbidden
    if (!(clientType & LGIVC_SYNC_READER)){
        IVC_LOG_ERROR("invalid clientType=%d.\n", clientType);
        return 1;
    }

    if (!buffer || !len) {
        IVC_LOG_ERROR("Invalid args buffer=%p len=%ld!!\n", buffer, len);
        return 1;
    }

    if(!reinterpret_cast<cMemoryManager*>(memManObj.get())->copy_from_memory(buffer, len, offset))
    {
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 0;
    }

    IVC_LOG_ERROR("%s failed\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
    return 1;
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
int32_t cIvcAcrnRW::pullData(void *buffer, const size_t len)
{
    int32_t ret = -1;
    int32_t status;
    ssize_t bytesAvailable = 0;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(rwState != LGIVC_RW_STARTINTERFACE_DONE)
    {
        IVC_LOG_ERROR("cIvcAcrnRW::startInterface was not set up\n");

        return 1;
    }

    if (!buffer || !len) {
        IVC_LOG_ERROR("Invalid args buffer=%p len=%ld!!\n", buffer, len);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 1;
    }

    //This API can be called by only sync reader or async reader with eventcallback registered. Others are forbidden
    if ((clientType & LGIVC_SYNC_READER) || ((clientType & LGIVC_ASYNC_READER) && (clientType & LGIVC_WITH_EVENTCALLBACK)))
    {
        status = reinterpret_cast<cMemoryManager*>(memManObj.get())->get_bytes_available(bytesAvailable);
        if (status || bytesAvailable < len) {
            IVC_LOG_ERROR("get_bytes_available returned=%d with bytesAvailable=%ld but req len=%ld!!\n", status, bytesAvailable, len);
            return 1;
        }

        ret = reinterpret_cast<cMemoryManager*>(memManObj.get())->copy_from_memory(buffer, len);
        if(ret == 0)
        {
            ret = reinterpret_cast<cMemoryManager*>(memManObj.get())->acknowledge_read();
        }
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return (ret == 0) ? 0 : 1;
    }
    else
    {
        IVC_LOG_ERROR("invalid clientType=%d.\n", clientType);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 1;
    }
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
int32_t cIvcAcrnRW::read(void *buffer, const size_t len)
{
    int32_t ret = -1;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(rwState != LGIVC_RW_STARTINTERFACE_DONE)
    {
        IVC_LOG_ERROR("cIvcAcrnRW::startInterface was not set up\n");

        return 1;
    }

    if (!buffer || !len) {
        IVC_LOG_ERROR("Invalid args buffer=%p len=%ld!!\n", buffer, len);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 1;
    }

    //This API can be called by only sync reader or async reader with eventcallback registered. Others are forbidden
    if ((clientType & LGIVC_SYNC_READER) || ((clientType & LGIVC_ASYNC_READER) && (clientType & LGIVC_WITH_EVENTCALLBACK)))
    {
        ret = reinterpret_cast<cMemoryManager*>(memManObj.get())->copy_from_memory(buffer, len);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return (ret == 0) ? 0 : 1;
    }
    else
    {
        IVC_LOG_ERROR("invalid clientType=%d.\n", clientType);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 1;
    }
}

/**************************************************************************************
 * Check for number of clients attached to a shared memory
 *
 * @param out ->  clientsAttached: number of clients attached to shared memory
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcAcrnRW::getNumberOfClientsAttached(int32_t &clientsAttached)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    clientsAttached = clientCount;

    return 0;
}


/**************************************************************************************
 * Get the client id connected to shared memory
 *
 * @param out ->  clientId: Id of client attached to shared memory
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cIvcAcrnRW::getClientId(int32_t &clientId)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    reinterpret_cast<cMemoryManager*>(memManObj.get())->get_shMemConnectionId(clientId);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}

