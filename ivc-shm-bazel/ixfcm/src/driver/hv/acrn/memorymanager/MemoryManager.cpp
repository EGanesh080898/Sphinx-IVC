#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "MemoryObject.hpp"
#include "MemoryManager.hpp"
#include "lgivc_shmem_hal.hpp"

std::mutex cMemoryManager::mlock;
sortedMap cMemoryManager::memoryMap;


std::string mFormatBytes(uint64_t bytes) {
    std::string units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size = bytes;
    while (size >= 1024 && i < sizeof(units)/sizeof(units[0])) {
        size /= 1024;
        i++;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[i];
    return oss.str();
}


/**************************************************************************************
 * Constructor
 * create Memory Object
 **************************************************************************************/
cMemoryManager::cMemoryManager()
{
    memObj = std::make_shared<cMemoryObj>();
    if(populateSharedMemoryInfo())
    {
        IVC_LOG_ERROR("populateSharedMemoryInfo failed\n");
    }
}


/**************************************************************************************
 * Destructor
 **************************************************************************************/
cMemoryManager::~cMemoryManager()
{
}

/**************************************************************************************
 * Populate information about all the shared memories available on the system.
 *
 *
 * @return 0: SUCCESSFULLY populated shared memory information.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::populateSharedMemoryInfo(void)
{
    ssize_t shmemSize = 0;
    uint32_t shMemNo = 0;
    bool memInUse = false;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    lgivc_shmem_hal_getNumberOfSharedMemory(shMemNo);
    if(!shMemNo)
    {
        IVC_LOG_ERROR("No shared memories available\n");
        return 1;
    }

    //Loop through all the shared memories and insert it to sorted set
    for(int i=0; i<shMemNo; i++)
    {
        std::shared_ptr<memoryInfo> mInfo = std::make_shared<memoryInfo>();
        if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_getShmemSize(i, shmemSize) && shmemSize > 0)
        {
            if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_checkMemoryStatus(i, "TestInUse", shmemSize, memInUse))
            {
                mInfo->uioDev = i;
                mInfo->shmemSize = shmemSize;
                mInfo->inUse = memInUse;
            }
            else
            {
                IVC_LOG_ERROR("%s Failed to get memory status\n",__FUNCTION__);
                mInfo->uioDev = i;
                mInfo->shmemSize = 0;
                mInfo->inUse = false;
            }
            IVC_LOG_DEBUG("Insert memory @uio%d of size=%ld, inuse=%d\n", mInfo->uioDev, mInfo->shmemSize, mInfo->inUse);
            memoryMap.insert(mInfo);
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}


/**************************************************************************************
 * Update information about all the shared memories available on the system.
 *
 *
 * @return 0: SUCCESSFULLY populated shared memory information.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::updateSharedMemoryInfo(void)
{
    ssize_t shmemSize = 0;
    uint32_t shMemNo = 0;
    bool memInUse = false;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    for(auto &memNode : memoryMap)
    {
        if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_checkMemoryStatus(memNode->uioDev, "TestInUse", memNode->shmemSize, memInUse))
        {
            memNode->inUse = memInUse;
        }
        else
        {
            IVC_LOG_ERROR("%s Failed to get memory status\n",__FUNCTION__);
            memNode->inUse = memInUse;
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}


/**************************************************************************************
 * Get the uio number of free memory
 *
 *
 * @return 0: SUCCESSFULLY found free shared memory.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::getNextFreeMemory(uint32_t &uioDevNode, const ssize_t &reqMemSize)
{
    ssize_t shmemSize = 0;
    uint32_t shMemNo = 0;
    bool memInUse = false;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

#if 0
    if(updateSharedMemoryInfo())
    {
        IVC_LOG_ERROR("updateSharedMemoryInfo failed\n");
        return 1;
    }
#endif
    //Loop through all the shared memories and see if it matches the requirement
    for(auto &memNode : memoryMap)
    {
        IVC_LOG_DEBUG("Check memory @uio%d of size=%s. Request size=%s, inuse=%d\n", memNode->uioDev, mFormatBytes(memNode->shmemSize).c_str(), mFormatBytes(reqMemSize).c_str(), memNode->inUse);
        if((false == memNode->inUse) && (reqMemSize <= memNode->shmemSize))
        {
            if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_checkMemoryStatus(memNode->uioDev, "TestInUse", memNode->shmemSize, memInUse))
            {
                if(false == memInUse)
                {
                    IVC_LOG_INFO("Found free memory @uio%d of size=%s. Request size=%s\n", memNode->uioDev, mFormatBytes(memNode->shmemSize).c_str(), mFormatBytes(reqMemSize).c_str());
                    uioDevNode = memNode->uioDev;
                    return 0;
                }
            }
            else
            {
                IVC_LOG_ERROR("%s Failed to get memory status\n",__FUNCTION__);
            }
        }
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 1;
}


/**************************************************************************************
 * Try attaching to any of the shared memories which has been already attached by other application having request name and size.
 *
 * @param in -> memName: name given to shared memory
 * @param in -> reqMemSize: requested shared memory size.
 *
 * @return 0: SUCCESSFULLY attached to shared memory.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::attachToExistingMemory(std::string memName, const ssize_t &reqMemSize)
{
    ssize_t shmemSize = 0;
    uint32_t shMemNo = 0;
    cMemoryObj *p = reinterpret_cast<cMemoryObj *>(memObj.get());

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    lgivc_shmem_hal_getNumberOfSharedMemory(shMemNo);

    //Loop through all the shared memories and see if it matches the memory name and size
    for(int i=0; i<shMemNo; i++)
    {
        if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_getShmemSize(i, shmemSize) && (shmemSize >= reqMemSize))
        {
            if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_attachToExistingMemory(i, memName, reqMemSize, &p->memory.shMemory, LGIVC_SHMEM_HAL_READ_WRITE, p->memory.connectionId, p->memory.virtualMachineId))
            {
                p->memory.devInstance = i;
                p->memory.memName = memName;
                p->memory.memSize = reqMemSize;
                if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_open(i, p->memory.clientFd))
                {
                    if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_registerIrq(p->memory.clientFd, p->memory.eventFd, p->memory.epollFd))
                    {
                        IVC_LOG_ENTEXT("%s Exit success\n",__FUNCTION__);
                        return 0;
                    }
                    else
                    {
                        IVC_LOG_DEBUG("lgivc_shmem_hal_registerIrq failed\n");
                        goto regIrqFailed;
                    }
                }
                else
                {
                    IVC_LOG_DEBUG("lgivc_shmem_hal_open failed\n");
                    goto devOpenFailed;
                }
regIrqFailed:
                lgivc_shmem_hal_close(p->memory.clientFd);
devOpenFailed:
                lgivc_shmem_hal_detach(&p->memory.shMemory, reqMemSize);
            }
            else
            {
                IVC_LOG_DEBUG("lgivc_shmem_hal_attach failed\n");
            }
        }
        else
        {
            IVC_LOG_DEBUG("lgivc_shmem_hal_getShmemSize failed OR sharedmemory size =%ld is less than requested size=%ld\n", shmemSize, reqMemSize);
        }
    }

    IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
    return 1;
}


/**************************************************************************************
 * Try attaching to any of the shared memories which has been allocated to any application.
 *
 * @param in -> memName: name given to shared memory
 * @param in -> reqMemSize: requested shared memory size.
 *
 * @return 0: SUCCESSFULLY attached to shared memory.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::allocMemory(std::string memName, const ssize_t &reqMemSize)
{
    uint32_t shMemNo = 0;
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(!getNextFreeMemory(shMemNo, reqMemSize))
    {
        if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_attach(shMemNo, memName, reqMemSize, &p->memory.shMemory, LGIVC_SHMEM_HAL_READ_WRITE, p->memory.connectionId, p->memory.virtualMachineId))
        {
            p->memory.devInstance = shMemNo;
            p->memory.memName = memName;
            p->memory.memSize = reqMemSize;
            //Open memory and register Irq
            if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_open(shMemNo, p->memory.clientFd))
            {
                if(LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_registerIrq(p->memory.clientFd, p->memory.eventFd, p->memory.epollFd))
                {
                    IVC_LOG_ENTEXT("%s Exit success\n",__FUNCTION__);
                    return 0;
                }
                else
                {
                    IVC_LOG_ERROR("lgivc_shmem_hal_registerIrq failed\n");
                    goto regIrqFailed;
                }
            }
            else
            {
                IVC_LOG_ERROR("lgivc_shmem_hal_open failed\n");
                goto devOpenFailed;
            }
regIrqFailed:
            lgivc_shmem_hal_close(p->memory.clientFd);
devOpenFailed:
            lgivc_shmem_hal_detach(&p->memory.shMemory, reqMemSize);
        }
        else
        {
            IVC_LOG_ERROR("lgivc_shmem_hal_attach failed\n");
        }
    }
    else
    {
        IVC_LOG_ERROR("No free shared memory available\n");
    }

    IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
    return 1;
}


/**************************************************************************************
 * Try attaching to existing shared memory if present with the same name and size.
 * If not present, attach to new shared memory.
 *
 * @param in -> memName: name given to shared memory
 * @param in -> reqMemSize: requested shared memory size.
 *
 * @return 0: SUCCESSFULLY attached to shared memory.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::getSharedMemory(std::string memName, const ssize_t &memSize)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mlock);

    if(!attachToExistingMemory(memName, memSize))
    {
        IVC_LOG_INFO("%s Memory Attached successfully to %s\n",__FUNCTION__, memName.c_str());
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 0;
    }

    if(!allocMemory(memName, memSize))
    {
        IVC_LOG_INFO("%s Memory Allocated successfully to %s\n",__FUNCTION__, memName.c_str());
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return 0;
    }

    IVC_LOG_ERROR("%s Failed to allocate shared memory for %s\n",__FUNCTION__, memName.c_str());
    IVC_LOG_ENTEXT("%s Exit Failed\n",__FUNCTION__);
    return 1;
}


/**************************************************************************************
 * Release shared memory.
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::releaseSharedMemory()
{
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mlock);
    lgivc_shmem_hal_deregisterIrq(p->memory.eventFd, p->memory.epollFd);
    lgivc_shmem_hal_detach(&p->memory.shMemory, p->memory.memSize);
    lgivc_shmem_hal_close(p->memory.clientFd);

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}


/**************************************************************************************
 * Notify clients attached to shared memory
 *
 * @param in -> clientList: list of clients to be notified
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::notifyClients(std::vector<int32_t> &clientList)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    for(auto &client : clientList)
    {
        if(LGIVC_SHMEM_HAL_SUCCESS != lgivc_shmem_hal_notifyClient(reinterpret_cast<cMemoryObj*>(memObj.get())->memory.devInstance, client))
        {
            IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
            return 1;
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}

/**************************************************************************************
 * Get list of clients attached to shared memory
 *
 * @param out -> clientList: list of clients to be notified
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::getClientList(std::vector<int32_t> &clientList)
{
    uint32_t attachVms = 0;
    uint32_t totalVmCount = 0;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    lgivc_shmem_hal_getNumberOfVMs(totalVmCount);

    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());
    if((LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_getAttachedVMs(&p->memory.shMemory, attachVms)) && attachVms)
    {
        /*TODO: Send interrupts to everybody, other than the current VM. Check if its fine.
         *      Comment below if you are woking on intraVM
         */
        attachVms &= ~(1 << p->memory.virtualMachineId);
        IVC_LOG_DEBUG("attachVms:%d totalVmCount=%d\n", attachVms, totalVmCount);
        for(int i=0; i<totalVmCount; i++)
        {
            if(attachVms & (1<<i))
                clientList.push_back(i);
        }
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}


/**************************************************************************************
 * Check for number of clients attached to a shared memory
 *
 * @param out ->  clientsAttached: number of clients attached to shared memory
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::getNumberOfClientsAttached(int32_t &clientsAttached)
{
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());
    if((LGIVC_SHMEM_HAL_SUCCESS == lgivc_shmem_hal_getNumberOfClientsAttached((const void **)&p->memory.shMemory, clientsAttached)))
    {
        IVC_LOG_DEBUG("cMemoryManager::getNumberOfClientsAttached success\n");

        return 0;
    }

    return 1;
}


/**************************************************************************************
 * Get client list and notify clients attached to shared memory
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::notifyClients()
{
    std::vector<int32_t> clientList;
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    //Get Clients attached
    if(!getClientList(clientList))
    {
        return notifyClients(clientList);
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 1;
}

/**************************************************************************************
 * Get direct access to memory object
 *
 * @return memObj: reference to shared memory object.
 **************************************************************************************/
std::shared_ptr<void> cMemoryManager::getMemoryObject()
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return memObj;
}


/**************************************************************
 * copy to shared Memory
 * 
 * @param in: buffer: buffer from which data needs to be copied
 * @param in: size: size of data to be copied
 * 
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************/
int32_t cMemoryManager::copy_to_memory(const void *buffer, const ssize_t &size)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    IVC_LOG_DEBUG("Len=%ld, buffer=%p\n", size, buffer);
    return (reinterpret_cast<cMemoryObj*>(memObj.get())->copy_to_memory(buffer, size) == true) ? 0 : 1;
}


/**************************************************************
 * copy to shared Memory at given offset
 * 
 * @param in: buffer: buffer from which data needs to be copied
 * @param in: size: size of data to be copied
 * @param in: offset: to where data is written
 * 
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************/
int32_t cMemoryManager::copy_to_memory(const void *buffer, const ssize_t &size, int32_t offset)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    IVC_LOG_DEBUG("Len=%ld, buffer=%p, Offset=%ld\n", size, buffer, offset);
    return (reinterpret_cast<cMemoryObj*>(memObj.get())->copy_to_memory(buffer, size, offset) == true) ? 0 : 1;
}



/**************************************************************
 * copy from shared Memory
 * 
 * @param out: buffer: buffer to which data needs to be copied
 * @param in: size: size of data to be copied
 * 
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************/
int32_t cMemoryManager::copy_from_memory(void *buffer, const ssize_t &size)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (reinterpret_cast<cMemoryObj*>(memObj.get())->copy_from_memory(buffer, size) == true) ? 0 : 1;
}



/**************************************************************
 * copy from shared Memory
 * 
 * @param out: buffer: buffer to which data needs to be copied
 * @param in: size: size of data to be copied
 * @param in: offset: from where data is read
 * 
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************/
int32_t cMemoryManager::copy_from_memory(void *buffer, const ssize_t &size, int32_t offset)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    IVC_LOG_DEBUG("Len=%ld, buffer=%p, Offset=%ld\n", size, buffer, offset);
    return (reinterpret_cast<cMemoryObj*>(memObj.get())->copy_from_memory(buffer, size, offset) == true) ? 0 : 1;
}

/**************************************************************************************
 * Wait for interrupt to be recived from other clients attached to shard memory
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::wait_for_data()
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (lgivc_shmem_hal_waitForData(p->memory.eventFd, p->memory.epollFd) == LGIVC_SHMEM_HAL_SUCCESS) ? 0 : 1;
}


/**************************************************************************************
 * Check for number of bytes received on shared memory
 *
 * @param out ->  bytes: bytes available for reading.
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::get_bytes_available(ssize_t &bytes)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (lgivc_shmem_hal_getDataAvailableForRead(&p->memory.shMemory, bytes) == LGIVC_SHMEM_HAL_SUCCESS) ? 0 : 1;
}


/**************************************************************************************
 * Check if all readers have read the data.
 *
 * @param out ->  status: 1 if all readers have read the data and 0 otherwise.
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::get_read_status(int32_t &status)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (lgivc_shmem_hal_readStatus(&p->memory.shMemory, status) == LGIVC_SHMEM_HAL_SUCCESS) ? 0 : 1;
}


/**************************************************************************************
 * Clear write information
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::clearMemory()
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (lgivc_shmem_hal_clearMemory(&p->memory.shMemory) == LGIVC_SHMEM_HAL_SUCCESS) ? 0 : 1;
}


/**************************************************************************************
 * Acknowledge the reading of data
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::acknowledge_read(void)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    cMemoryObj *p = reinterpret_cast<cMemoryObj*>(memObj.get());
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (lgivc_shmem_hal_acknowledgeRead(p->memory.devInstance, &p->memory.shMemory) == LGIVC_SHMEM_HAL_SUCCESS) ? 0 : 1;
}


/**************************************************************************************
 * get shared memory name
 *
 * @param out: shmemName: shared memory name
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::get_shMemName(std::string &shMemName)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (reinterpret_cast<cMemoryObj*>(memObj.get())->get_sharedmemory_name(shMemName) == true) ? 0 : 1;
}


/**************************************************************************************
 * get shared memory size
 *
 * @param out: shmemSize: shared memory size
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::get_shMemSize(ssize_t &shMemSize)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (reinterpret_cast<cMemoryObj*>(memObj.get())->get_sharedmemory_size(shMemSize) == true) ? 0 : 1;
}


/**************************************************************************************
 * get shared memory connection id
 *
 * @param out: shmemSize: shared memory size
 *
 * @return 0: SUCCESS.
 * @return 1 : FAILED.
 **************************************************************************************/
int32_t cMemoryManager::get_shMemConnectionId(int32_t &connId)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    std::unique_lock<std::mutex> lock(mlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (reinterpret_cast<cMemoryObj*>(memObj.get())->get_sharedmemory_connectionId(connId) == true) ? 0 : 1;
}

