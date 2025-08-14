/**************************************************************************************
 * lgivc_shmem_hal.c
 *
 **************************************************************************************/
#include <dirent.h>
#include "lgivc_shmem_hal.hpp"
#include "lgivc_shmem_hal_priv.hpp"

std::mutex mHalLock;

/**************************************************************************************
 * Get number of shared memories available on this platform
 *
 * @param out -> sharedMemoryNum: no of shared memories
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getNumberOfSharedMemory(uint32_t &sharedMemoryNum)
{
    uint32_t memCnt = 0;
    DIR *dirp = NULL;
    struct dirent *dp =NULL;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    dirp = opendir("/dev/");
    while (dirp) {
        if ((dp = readdir(dirp)) != NULL) {
            if (strstr(dp->d_name, "uio")) {
                memCnt++;
            }
        } else {
            closedir(dirp);
            goto err;
        }
    }
err:
    IVC_LOG_INFO("Number of shared memories detected = %d\n", memCnt);
    sharedMemoryNum = memCnt;
    if(!memCnt)
    {
        sharedMemoryNum = MAX_RESERVED_SHARED_MEMORY;
        IVC_LOG_WARNING("Setting Hardcoded number of shared memories = %d\n", memCnt);
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_SUCCESS;
}


/**************************************************************************************
 * Get number of userVMs available on this platform
 *
 * @param out -> vmCount: no of userVMs
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getNumberOfVMs(uint32_t &vmCount)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    vmCount = TOTAL_VM_COUNT;
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_SUCCESS;
}


/**************************************************************************************
 * Open memory device and provide fd
 *
 * @param in ->  devInstance: memory instance
 * @param out ->  devfd: device fd
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_open(const int32_t &devInstance, int32_t &devfd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    return (lgivc_shmem_hal_private_open(devInstance, devfd) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Deregister IRQ for the given shared memory.
 *
 * @param out -> evtfd: event fd
 * @param out -> epfd: epoll fd
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_deregisterIrq(const int32_t &evtfd, const int32_t &epfd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    return (lgivc_shmem_hal_private_deregisterIrq(evtfd, epfd) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Regiser IRQ for the given shared memory.
 * Interrupt is received whenever any application
 * attached to shared memory notifies us
 *
 * @param in -> devfd: device fd
 * @param out -> evtfd: event fd
 * @param out -> epfd: epoll fd
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_registerIrq(int32_t &devfd, int32_t &evtfd, int32_t &epfd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    return (lgivc_shmem_hal_private_registerIrq(devfd, evtfd, epfd) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Retrive the size of shared memory for the given instance.
 *
 * @param in -> devInstance: memory instance
 * @param out -> shm_size: size of shared memory
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getShmemSize(const int32_t &devInstance, ssize_t &shm_size)
{
    uint32_t actual_size = 0;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(lgivc_shmem_hal_private_getShmemSize(devInstance, actual_size) == true)
    {
        shm_size = actual_size - sizeof(lgivc_shmem_hal_meta);
        return LGIVC_SHMEM_HAL_SUCCESS;
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Try attaching to shared memory which has been already attached by other application.
 *
 * @param in -> devInstance: memory instance
 * @param in -> memName: name given to shared memory
 * @param in -> memorySize: requested shared memory size.
 * @param out -> memoryAddr: allocated shared memory address.
 * @param in -> mode: mode with which shared memory has to be opened.
 * @param out -> connId: unique shared memory id.
 * @param out -> vmId: virtual machine id.
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESSFULLY attached to shared memory.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAILED to attach.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_attachToExistingMemory(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, void **memAddr, const eLGIVC_SHMEM_HAL_MODES &mode, int32_t &connId, uint32_t &vmId)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    volatile lgivc_shmem_hal_meta *meta = NULL;
    ssize_t req_size = memorySize + sizeof(lgivc_shmem_hal_meta);
    int32_t flags = 0;
    int32_t prot = 0;
    uint32_t vmid = -1;

    if((false == lgivc_shmem_hal_private_getVmId(devInstance, vmid)) || (vmid < 0))
    {
        IVC_LOG_ERROR("%s Failed to get vmId!", __FUNCTION__);
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    if(!memName.length() || !memorySize || memName.length() >= MAX_SHM_NAME_SIZE)
    {
        IVC_LOG_ERROR("%s Invalid params!", __FUNCTION__);
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    switch(mode)
    {
        case LGIVC_SHMEM_HAL_READ_ONLY :
            flags = O_RDONLY;
            prot = PROT_READ;
            IVC_LOG_DEBUG("Read only mode\n");
            break;

        case LGIVC_SHMEM_HAL_WRITE_ONLY :
            flags = O_WRONLY;
            prot = PROT_WRITE;
            IVC_LOG_DEBUG("write only mode\n");
            break;

        case LGIVC_SHMEM_HAL_READ_WRITE :
            flags = O_RDWR;
            prot = PROT_READ | PROT_WRITE;
            IVC_LOG_DEBUG("Read and write mode\n");
            break;

        default :
            IVC_LOG_ERROR("Invalid mode_type\n");
            IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
            return LGIVC_SHMEM_HAL_FAILURE;
    }

    //Attach t shared memory and get address of it
    bool ret = lgivc_shmem_hal_private_attach(devInstance, memName, req_size, memAddr, flags, prot);

    if (true == ret) {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
        //Check if other clients have already attached to this shared memory
        if((MEMORY_IN_USE_SIGNATURE == meta->memInUse) && (meta->shMemSize == memorySize))
        {
            if(!strcmp(memName.c_str(), (char*)meta->shMemName))
            {
                lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
                meta->attachedVMs |= ((uint64_t)1 << vmid);
                meta->clientId++;
                meta->attachCount++;
                lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
#ifndef ACRN_ENABLE_IVCHEARTBEAT
                meta->clientCount++;
#endif
                connId = meta->clientId;
                vmId = vmid;
                IVC_LOG_DEBUG("%s Attached to existing memory @ %p with clients %ld\n", __FUNCTION__, meta, meta->clientId);
                return LGIVC_SHMEM_HAL_SUCCESS;
            }
        }
        else
        {
            lgivc_shmem_hal_private_detach(memAddr, req_size);
            IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
            return LGIVC_SHMEM_HAL_FAILURE;
        }
    }

    IVC_LOG_DEBUG("fd = %d memName =%s mesasge_size = %d\n", devInstance,memName, memorySize);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    return LGIVC_SHMEM_HAL_FAILURE;
}



/**************************************************************************************
 * Try attaching to shared memory which has been already attached by other application.
 *
 * @param in -> devInstance: memory instance
 * @param in -> memName: name given to shared memory
 * @param in -> memorySize: requested shared memory size.
 * @param out -> memoryAddr: allocated shared memory address.
 * @param in -> mode: mode with which shared memory has to be opened.
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESSFULLY attached to shared memory.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAILED to attach.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_attachToExistingMemory(const int32_t &devInstance, const std::string &memName, void **memAddr, const eLGIVC_SHMEM_HAL_MODES &mode)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    volatile lgivc_shmem_hal_meta *meta = NULL;
    ssize_t req_size = sizeof(lgivc_shmem_hal_meta);
    int32_t flags = 0;
    int32_t prot = 0;

    if(!memName.length() || memName.length() >= MAX_SHM_NAME_SIZE)
    {
        IVC_LOG_ERROR("%s Invalid params!", __FUNCTION__);
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    switch(mode)
    {
        case LGIVC_SHMEM_HAL_READ_ONLY :
            flags = O_RDONLY;
            prot = PROT_READ;
            IVC_LOG_DEBUG("Read only mode\n");
            break;

        case LGIVC_SHMEM_HAL_WRITE_ONLY :
            flags = O_WRONLY;
            prot = PROT_WRITE;
            IVC_LOG_DEBUG("write only mode\n");
            break;

        case LGIVC_SHMEM_HAL_READ_WRITE :
            flags = O_RDWR;
            prot = PROT_READ | PROT_WRITE;
            IVC_LOG_DEBUG("Read and write mode\n");
            break;

        default :
            IVC_LOG_ERROR("Invalid mode_type\n");
            IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
            return LGIVC_SHMEM_HAL_FAILURE;
    }

    //Attach t shared memory and get address of it
    bool ret = lgivc_shmem_hal_private_attach(devInstance, memName, req_size, memAddr, flags, prot);

    if (true == ret) {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
        //Check if other clients have already attached to this shared memory
        if(MEMORY_IN_USE_SIGNATURE == meta->memInUse)
        {
            if(!strcmp(memName.c_str(), (char*)meta->shMemName))
            {
                IVC_LOG_DEBUG("%s Attached to existing memory @ %p\n", __FUNCTION__, meta);
                return LGIVC_SHMEM_HAL_SUCCESS;
            }
        }
        else
        {
            lgivc_shmem_hal_private_detach(memAddr, req_size);
            IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
            return LGIVC_SHMEM_HAL_FAILURE;
        }
    }

    IVC_LOG_DEBUG("fd = %d memName =%s req_size = %d\n", devInstance,memName, req_size);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    return LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Request attachment to new shared memory.
 *
 * @param in -> devInstance: memory instance
 * @param in -> memName: name given to shared memory
 * @param in -> memorySize: requested shared memory size.
 * @param out -> memoryAddr: allocated shared memory address.
 * @param in -> mode: mode with which shared memory has to be opened.
 * @param out -> connId: unique shared memory id.
 * @param out -> vmId: virtual machine id.
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESSFULLY attached to shared memory.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAILED to attach.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_attach(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, void **memAddr, const eLGIVC_SHMEM_HAL_MODES &mode, int32_t &connId, uint32_t &vmId)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    ssize_t req_size = memorySize + sizeof(lgivc_shmem_hal_meta);
    int32_t flags = 0;
    int32_t prot = 0;
    uint32_t vmid = -1;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if((false == lgivc_shmem_hal_private_getVmId(devInstance, vmid)) || (vmid < 0))
    {
        IVC_LOG_ERROR("%s Failed to get vmId!", __FUNCTION__);
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    if(!memName.length() || !memorySize || memName.length() >= MAX_SHM_NAME_SIZE)
    {
        IVC_LOG_ERROR("%s Invalid params!", __FUNCTION__);
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    switch(mode)
    {
        case LGIVC_SHMEM_HAL_READ_ONLY :
            flags = O_RDONLY | O_NONBLOCK;
            prot = PROT_READ;
            IVC_LOG_DEBUG("Read only mode\n");
            break;

        case LGIVC_SHMEM_HAL_WRITE_ONLY :
            flags = O_WRONLY | O_NONBLOCK;
            prot = PROT_WRITE;
            IVC_LOG_DEBUG("write only mode\n");
            break;

        case LGIVC_SHMEM_HAL_READ_WRITE :
            flags = O_RDWR | O_NONBLOCK;
            prot = PROT_READ | PROT_WRITE;
            IVC_LOG_DEBUG("Read and write mode\n");
            break;

        default :
            IVC_LOG_ERROR("Invalid mode_type\n");
            IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
            return LGIVC_SHMEM_HAL_FAILURE;
    }

    //Attach to shared memory and get address of it
    bool ret = lgivc_shmem_hal_private_attach(devInstance, memName, req_size, memAddr, flags, prot);

    if (true == ret) {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
        //If no other clients have attached, update meta fields
        if(MEMORY_IN_USE_SIGNATURE != meta->memInUse)
        {
            int ret = -1;
            memset(meta, 0x0, sizeof(lgivc_shmem_hal_meta));
            ret = lgivc_shmem_hal_rwlock_init(&meta->rwlock);
            if(0 == ret)
            {
                lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
                strcpy(meta->shMemName, memName.c_str());
                meta->shMemSize = memorySize;
                meta->memInUse = MEMORY_IN_USE_SIGNATURE;
                meta->dataSize = 0;
                meta->readAckCount = 0;
                meta->clientId = 1;
                meta->attachCount = 1;
#ifdef ACRN_ENABLE_IVCHEARTBEAT
                meta->clientCount = 0;
#else
                meta->clientCount = 1;
#endif
                meta->attachedVMs = ((uint64_t)1 << vmid);
                meta->intendedClients = 0;
                meta->dirty = false;
                lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
                connId = meta->clientId;
                vmId = vmid;
                IVC_LOG_DEBUG("%s meta=%p %ld\n", __FUNCTION__, meta, sizeof(lgivc_shmem_hal_meta));
                return LGIVC_SHMEM_HAL_SUCCESS;
            }
            else
            {
                IVC_LOG_ERROR("%s lgivc_shmem_hal_rwlock_init failed\n", __FUNCTION__);
            }
        }
        else
        {
            lgivc_shmem_hal_private_detach(memAddr, req_size);
            IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
            return LGIVC_SHMEM_HAL_FAILURE;
        }
    }

    IVC_LOG_DEBUG("fd = %d, memName = %s, memorySize = %d\n", devInstance, memName.c_str(), memorySize);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    return LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Check if given shared memory is free.
 *
 * @param in -> devInstance: memory instance
 * @param in -> memName: name given to shared memory
 * @param in -> memorySize: requested shared memory size.
 * @param out -> useStatus: true if memory is in use and false otherwise
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESSFULLY attached to shared memory.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAILED to attach.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_checkMemoryStatus(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, bool &useStatus)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    int32_t flags = O_RDWR | O_NONBLOCK;
    int32_t prot = PROT_READ | PROT_WRITE;
    void *memAddr;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    //Attach to shared memory and get address of it
    bool ret = lgivc_shmem_hal_private_attach(devInstance, memName.c_str(), memorySize, &memAddr, flags, prot);
    if (true == ret) {
        meta = (lgivc_shmem_hal_meta *)memAddr;
        //If no other clients have attached, update meta fields
        useStatus = (MEMORY_IN_USE_SIGNATURE == meta->memInUse) ? true : false;
        lgivc_shmem_hal_private_detach(&memAddr, memorySize);
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_SUCCESS;
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    return LGIVC_SHMEM_HAL_FAILURE;
}



/**************************************************************************************
 * Read data from shared memory
 *
 * @param out: read_buffer: buffer in which data needs to be copied
 * @param in: in_size: size of data to be copied
 * @param out: memAddr: shared memory address
 * @param in: vmId: virtual machine id of current vm
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_read(void *read_buffer, ssize_t  in_size, void **memAddr, uint32_t vmId)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    void *readPtr = NULL;
    bool ret = false;
    volatile lgivc_shmem_hal_meta *meta = NULL;

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr == NULL)
    {
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }
    IVC_LOG_DEBUG("%s address=%p\n", __FUNCTION__, *memAddr);
    meta = (lgivc_shmem_hal_meta *)*memAddr;
    //lgivc_shmem_hal_rwlock_readlock(&meta->rwlock);
    if(true == meta->dirty.load())
    {
        //lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);
        //Skip to section where data needs to be read from
        readPtr = ((int8_t *)*memAddr) + sizeof(lgivc_shmem_hal_meta);
        ret = lgivc_shmem_hal_private_read(read_buffer, readPtr, in_size);
    }
    else
    {
        //lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (ret == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}



/**************************************************************************************
 * Read data from shared memory
 *
 * @param out: read_buffer: buffer in which data needs to be copied
 * @param in: in_size: size of data to be copied
 * @param out: memAddr: shared memory address
 * @param in: vmId: virtual machine id of current vm
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_read_from_offset(void *read_buffer, ssize_t  in_size, void **memAddr, uint32_t vmId, int32_t offset)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    void *readPtr = NULL;
    bool ret = false;
    volatile lgivc_shmem_hal_meta *meta = NULL;

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr == NULL)
    {
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }
    IVC_LOG_DEBUG("%s address=%p\n", __FUNCTION__, *memAddr);
    meta = (lgivc_shmem_hal_meta *)*memAddr;
    lgivc_shmem_hal_rwlock_readlock(&meta->rwlock);
    if(true == meta->dirty)
    {
        lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);
        //Skip to section where data needs to be read from
        readPtr = ((int8_t *)*memAddr) + sizeof(lgivc_shmem_hal_meta) + offset;
        ret = lgivc_shmem_hal_private_read(read_buffer, readPtr, in_size);
    }
    else
    {
        lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (ret == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Write data to shared memory
 *
 * @param out: write_buffer: buffer from which data needs to be copied
 * @param in: in_size: size of data to be copied
 * @param out: memAddr: shared memory address
 * @param in: vmId: virtual machine id of current vm
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_write(const void *write_buffer, const ssize_t &in_size, void **memAddr, const uint32_t &vmId)
{
    void *writePtr = NULL;
    volatile lgivc_shmem_hal_meta *meta = NULL;
    bool ret = false;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr == NULL)
    {
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    IVC_LOG_DEBUG("%s address=%p\n", __FUNCTION__, *memAddr);
    meta = (lgivc_shmem_hal_meta *)*memAddr;
    lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
    //Update meta information
    meta->dataSize = in_size;
    meta->readAckCount = 0;
    meta->writerVM = vmId;
    meta->intendedClients = meta->clientCount - 1;
    meta->dirty = true;
    lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
    //Skip to section where data needs to be written
    writePtr = ((int8_t *)*memAddr) + sizeof(lgivc_shmem_hal_meta);

    ret = lgivc_shmem_hal_private_write(write_buffer, writePtr, in_size);

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (ret == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Write data to shared memory at offset
 *
 * @param out: write_buffer: buffer from which data needs to be copied
 * @param in: in_size: size of data to be copied
 * @param out: memAddr: shared memory address
 * @param in: vmId: virtual machine id of current vm
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_write_to_offset(const void *write_buffer, const ssize_t &in_size, void **memAddr, const uint32_t &vmId, int32_t offset)
{
    void *writePtr = NULL;
    volatile lgivc_shmem_hal_meta *meta = NULL;
    bool ret = false;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr == NULL)
    {
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    IVC_LOG_DEBUG("%s address=%p\n", __FUNCTION__, *memAddr);
    meta = (lgivc_shmem_hal_meta *)*memAddr;
    lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
    //Update meta information
    meta->dataSize = in_size;
    meta->readAckCount = 0;
    meta->writerVM = vmId;
    meta->intendedClients = meta->clientCount;
    meta->dirty = true;
    lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
    //Skip to section where data needs to be written
    writePtr = ((int8_t *)*memAddr) + sizeof(lgivc_shmem_hal_meta) + offset;

    ret = lgivc_shmem_hal_private_write(write_buffer, writePtr, in_size);

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return (ret == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Notify clients attached to shared memory
 *
 * @param in ->  devInstance: memory instance
 * @param in ->  clientId: id of client to be notified
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_notifyClient(const int32_t &devInstance, const int32_t &clientId)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    return (lgivc_shmem_hal_private_notifyClient(devInstance, clientId) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}

/**************************************************************************************
 * Acknowledge the reading of data by incrementing read count.
 *
 * @param in ->  devInstance: memory instance
 * @param in ->  memAddr: shared memory address
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_acknowledgeRead(const int32_t &devInstance, void **memAddr)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr == NULL)
    {
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    IVC_LOG_DEBUG("%s address=%p\n", __FUNCTION__, *memAddr);
    meta = (lgivc_shmem_hal_meta *)*memAddr;
    //lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
    meta->readAckCount++;
    //lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
    lgivc_shmem_hal_private_notifyClient(devInstance, meta->writerVM);

    IVC_LOG_DEBUG("Read status count=%x\n", meta->readAckCount.load());
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_SUCCESS;
}

/**************************************************************************************
 * Check if all readers have read the data.
 *
 * @param in ->  memAddr: shared memory address
 * @param out ->  status: 1 if all readers have read the data and 0 otherwise.
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_readStatus(void **memAddr, int32_t &status)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr == NULL)
    {
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    meta = (lgivc_shmem_hal_meta *)*memAddr;
    IVC_LOG_DEBUG("%s: dirty=%d ic=%ld rac=%ld\n", __FUNCTION__, meta->dirty.load(), meta->intendedClients, meta->readAckCount.load());
    lgivc_shmem_hal_rwlock_readlock(&meta->rwlock);
    if((meta->dirty == true) && (meta->intendedClients == meta->readAckCount))
    {
        status = 1;
    }
    lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);

    IVC_LOG_DEBUG("Read status=%x\n", status);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_SUCCESS;
}


/**************************************************************************************
 * Clear write meta fields
 *
 * @param in ->  memAddr: shared memory address
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_clearMemory(void **memAddr)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr == NULL)
    {
        IVC_LOG_ENTEXT("%s Exit failure\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_FAILURE;
    }

    meta = (lgivc_shmem_hal_meta *)*memAddr;

    meta->dirty = false;
    meta->intendedClients = 0;
    meta->readAckCount = 0;

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_SUCCESS;
}


/**************************************************************************************
 * Wait for interrupt to be recived from other clients attached to shard memory
 *
 * @param in -> evtfd: event fd
 * @param in -> epfd: epoll fd
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : on receiving interrupt.
 * @return LGIVC_SHMEM_HAL_FAILURE : otherwise.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_waitForData(const int32_t &evtfd, const int32_t &epfd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    //std::unique_lock<std::mutex> lock(mHalLock);

    return (lgivc_shmem_hal_private_waitForData(evtfd, epfd) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Check for number of bytes received on shared memory
 *
 * @param in ->  memAddr: shared memory address
 * @param out ->  bytesAvailable: bytes available for reading.
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getDataAvailableForRead(void **memAddr, ssize_t &bytesAvailable)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    meta = (lgivc_shmem_hal_meta *)*memAddr;
    if(meta && (true == meta->dirty) && (meta->intendedClients > meta->readAckCount))
    {
        //lgivc_shmem_hal_rwlock_readlock(&meta->rwlock);
        bytesAvailable = meta->dataSize.load();
        //lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_SUCCESS;
    }
    bytesAvailable = 0;
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Check for number of VMs attached to a shared memory
 *
 * @param in ->  memAddr: shared memory address
 * @param out ->  attachedVMs: number of VMs attached to shared memory
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getAttachedVMs(void **memAddr, uint32_t &attachedVMs)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    meta = (lgivc_shmem_hal_meta *)*memAddr;
    if(meta)
    {
        lgivc_shmem_hal_rwlock_readlock(&meta->rwlock);
        attachedVMs = meta->attachedVMs;
        lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);
        IVC_LOG_DEBUG("attached VMs=%x\n", attachedVMs);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return LGIVC_SHMEM_HAL_SUCCESS;
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Check for number of clients attached to a shared memory
 *
 * @param in ->  memAddr: shared memory address
 * @param out ->  clientsAttached: number of clients attached to shared memory
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getNumberOfClientsAttached(const void **memAddr, int32_t &clientsAttached)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    meta = (lgivc_shmem_hal_meta *)*memAddr;
    if(meta)
    {
        //lgivc_shmem_hal_rwlock_readlock(&meta->rwlock);
        clientsAttached = meta->clientCount.load();
        //lgivc_shmem_hal_rwlock_readunlock(&meta->rwlock);
        IVC_LOG_DEBUG("meta->clientCount = %d\n", clientsAttached);
        return LGIVC_SHMEM_HAL_SUCCESS;
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}

/**************************************************************************************
 * Increment Client Count
 *
 * @param in ->  memAddr: shared memory address
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_increment_clientCount(void **memAddr)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr)
    {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
        if(MEMORY_IN_USE_SIGNATURE == meta->memInUse)
        {
            meta->clientCount++;
            return LGIVC_SHMEM_HAL_SUCCESS;
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}

/**************************************************************************************
 * Decrement Client Count
 *
 * @param in ->  memAddr: shared memory address
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_decrement_clientCount(void **memAddr)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr)
    {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
        if(MEMORY_IN_USE_SIGNATURE == meta->memInUse)
        {
            meta->clientCount--;
            return LGIVC_SHMEM_HAL_SUCCESS;
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}

/**************************************************************************************
 * Set VMs Information
 *
 * @param in ->  memAddr: shared memory address
 * @param in ->  attachedVMs: attached VMs info
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_setVMsInfo(void **memAddr, std::vector<uint32_t> &attachedVMs)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr)
    {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
        if(MEMORY_IN_USE_SIGNATURE == meta->memInUse)
        {
            lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
            for(auto &vmid : attachedVMs)
                meta->attachedVMs |= ((uint64_t)1 << vmid);
            lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
            return LGIVC_SHMEM_HAL_SUCCESS;
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}


/**************************************************************************************
 * Get VMs Information
 *
 * @param in ->  memAddr: shared memory address
 * @param in ->  attachedVMs: attached VMs info
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getVMsInfo(void **memAddr, std::vector<uint32_t> &attachedVMList)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    uint64_t attachedVMs;

    std::unique_lock<std::mutex> lock(mHalLock);

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    if(*memAddr)
    {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
        if(MEMORY_IN_USE_SIGNATURE == meta->memInUse)
        {
            lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
            attachedVMs = meta->attachedVMs;
            lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
            uint64_t vmMask = 1;
            for(uint32_t vmId=0; vmId<sizeof(attachedVMs)*8; vmId++)
            {
                if(attachedVMs & vmMask)
                {
                    attachedVMList.push_back(vmId);
                }
                vmMask <<= 1;
            }
            return LGIVC_SHMEM_HAL_SUCCESS;
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}

/**************************************************************************************
 * Detach from shared memory
 *
 * @param in ->  memAddr: shared memory address
 * @param in ->  shmem_size: size of shared memory
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_detach(void **memAddr, const ssize_t &shmem_size)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    //uint64_t clientCount;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr)
    {
        meta = (lgivc_shmem_hal_meta *)*memAddr;
#ifndef ACRN_ENABLE_IVCHEARTBEAT
        meta->clientCount--;
#endif
        meta->attachCount--;
        //lgivc_shmem_hal_rwlock_writelock(&meta->rwlock);
        //clientCount = meta->clientCount.load();
        if(meta->attachCount.load() <= 0)
        {
            meta->memInUse = 0;
        }
        //lgivc_shmem_hal_rwlock_writeunlock(&meta->rwlock);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return (lgivc_shmem_hal_private_detach(memAddr, shmem_size) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}

/**************************************************************************************
 * Detach from shared memory
 *
 * @param in ->  memAddr: shared memory address
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 **************************************************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_detach(void **memAddr)
{
    volatile lgivc_shmem_hal_meta *meta = NULL;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    if(*memAddr)
    {
        ssize_t shmem_size = sizeof(lgivc_shmem_hal_meta);
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return (lgivc_shmem_hal_private_detach(memAddr, shmem_size) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return LGIVC_SHMEM_HAL_FAILURE;
}

/****************************************************
 * Close the shared memory instance
 *
 * @param in ->  devfd: device fd
 *
 * @return LGIVC_SHMEM_HAL_SUCCESS : SUCCESS.
 * @return LGIVC_SHMEM_HAL_FAILURE : FAIL.
 ****************************************************/
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_close(const int32_t &devFd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    std::unique_lock<std::mutex> lock(mHalLock);

    return (lgivc_shmem_hal_private_close(devFd) == true) ? LGIVC_SHMEM_HAL_SUCCESS : LGIVC_SHMEM_HAL_FAILURE;
}

