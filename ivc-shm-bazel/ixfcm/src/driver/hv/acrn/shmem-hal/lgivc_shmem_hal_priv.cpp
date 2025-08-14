/**************************************************************************************
 * lgivc_shmem_hal_priv.c
 *
 **************************************************************************************/
#include "lgivc_shmem_hal_priv.hpp"


/****************************************************
 * Get Device node string.
 *
 * @param in ->  devInstance: device instance
 * @return dev node node as char *.
 *
 ****************************************************/
static int8_t* getDeviceNode(const int32_t &devInstance)
{
    static int8_t devNode[100] = {0};

    sprintf((char*)devNode, SHMEM_DEVICE , devInstance);

    return devNode;
}


/****************************************************
 * Get config node string.
 *
 * @param in ->  devInstance: device instance
 * @return config node as char *.
 *
 ****************************************************/
static int8_t* getConfigDevice(const int32_t &devInstance)
{
    static int8_t cfgDev[100] = {0};

    sprintf((char*)cfgDev, SHMEM_CONFIG_DEVICE , devInstance);

    return cfgDev;
}


/****************************************************
 * Get Bar0 node string.
 *
 * @param in ->  devInstance: device instance
 * @return bar0 node as char *.
 *
 ****************************************************/
static int8_t* getBar0Dev(const int32_t &devInstance)
{
    static int8_t barDev[100] = {0};

    sprintf((char*)barDev, SHMEM_BAR0_DEVICE , devInstance);

    return barDev;
}


/****************************************************
 * Get Bar2 Cache string.
 *
 * @param in ->  devInstance: device instance
 * @return bar2 cache as char *.
 *
 ****************************************************/
static int8_t* getBar2Cache(const int32_t &devInstance)
{
    static int8_t barDev[100] = {0};

    sprintf((char*)barDev, SHMEM_BAR2_CACHE_WC_DEVICE , devInstance);

    return barDev;
}


/****************************************************
 * Open memory Device and provide fd
 *
 * @param in -> devInstance: dev instance
 * @param out -> devfd: memory fd
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_open(const int32_t &devInstance, int32_t &devfd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_DEBUG("devNode: %s\n", getDeviceNode(devInstance));
    devfd = open((char*)getDeviceNode(devInstance), O_RDWR);
    IVC_LOG_DEBUG("devInstance = %d\n", devInstance);

    if(devfd <= 0)
    {
        IVC_LOG_ERROR("SHM Device open failed\n");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return false;
    }

    IVC_LOG_DEBUG("SHM Device open success\n");
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}

/****************************************************
 * Register interrupt for shared memory device
 *
 * @param in -> devfd: memory fd
 * @param out -> evtfd: event fd
 * @param out -> epfd: epoll fd
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_registerIrq(int32_t &devfd, int32_t &evtfd, int32_t &epfd)
{
    struct epoll_event events = {0};
    struct uio_irq_data irq_data;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(devfd <= 0)
    {
        IVC_LOG_ERROR("invalid devfd value\n");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return false;
    }
    else
    {
        evtfd = eventfd(0, 0);
        if (evtfd < 0) {
            IVC_LOG_ERROR("failed to create evtfd\n");
            return false;
        }
        /* set eventfds of msix to kernel driver by ioctl */
        irq_data.vector = 0;
        irq_data.fd = evtfd;
        if (ioctl(devfd, UIO_IRQ_DATA, &irq_data) < 0) {
            IVC_LOG_ERROR("ioctl(UIO_IRQ_DATA) failed:\n");
            return false;
        }
        IVC_LOG_DEBUG("ioctl(UIO_IRQ_DATA) fd=%d\n", evtfd);

        /* create epoll */
        epfd = epoll_create1(0);
        if (epfd < 0) {
            IVC_LOG_ERROR("failed to epoll create\n");
            return false;
        }

        /* add eventfds of msix to epoll */
        events.events = EPOLLIN;
        events.data.ptr = &irq_data;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, evtfd, &events) < 0) {
            IVC_LOG_ERROR("epoll_ctl EPOLL_CTL_ADD failed\n");
            return false;
        }
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


/****************************************************
 * Deregister interrupt for shared memory device
 *
 * @param in -> devfd: memory fd
 * @param out -> evtfd: event fd
 * @param out -> epfd: epoll fd
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_deregisterIrq(const int32_t &evtfd, const int32_t &epfd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(evtfd > 0)
    {
        close(evtfd);
    }
    if(epfd > 0)
    {
        close(epfd);
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


/****************************************************
 * Retrive the size of shared memory for the given instance.
 *
 * @param in -> devInstance: memory instance
 * @param out -> shm_size: size of shared memory
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_getShmemSize(const int32_t &devInstance, uint32_t &shm_size)
{
    int32_t configfd = -1;
    uint64_t tmp;
    uint32_t size;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    configfd = open((char*)getConfigDevice(devInstance), O_RDWR);

    if(configfd <= 0)
    {
        IVC_LOG_ERROR("SHM open failed\n");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return false;
    }
    pread(configfd, &tmp, 8, 0x18);
    size= ~0U;
    pwrite(configfd ,&size, 4, 0x18);
    pread(configfd, &size, 4, 0x18);
    pwrite(configfd ,&tmp, 8, 0x18);
    size &= (~0xfUL);
    shm_size = (size & ~(size - 1));
    close(configfd);

    return true;
}


/****************************************************
 * Open the bar2 cache and mmap the shared memory to userspace.
 *
 * @param in -> devInstance: memory instance
 * @param in -> memName: name given to shared memory
 * @param in -> memorySize: requested shared memory size.
 * @param out -> memoryAddr: allocated shared memory address.
 * @param in -> flags: flag for device open.
 * @param in -> prot: prot for mmap.
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_attach(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, void **memAddr, const int32_t &flags, const int32_t &prot)
{
    int32_t bar2fd = -1;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(devInstance < 0)
    {
        IVC_LOG_ERROR("invalid devInstance value\n");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return false;
    }

    if(memorySize > 0)
    {
        bar2fd = open((char*)getBar2Cache(devInstance), flags);

        if(bar2fd <= 0)
        {
            IVC_LOG_ERROR("SHM open failed\n");
            IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
            return false;
        }

       *memAddr = mmap(NULL, memorySize, prot, MAP_SHARED, bar2fd, 0);
        if (*memAddr == MAP_FAILED) {
            close(bar2fd);
            IVC_LOG_ERROR("bar2_fd mmap failed\n");
            return false;
        }
        close(bar2fd);
    }
    IVC_LOG_DEBUG("%s mmap addr=%p\n", __FUNCTION__, *memAddr);

    IVC_LOG_DEBUG("fd = %d memName =%s memorySize = %d\n", devInstance,memName, memorySize);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    return true;
}


/****************************************************
 * Read data from shared memory
 *
 * @param out: read_buffer: buffer in which data needs to be copied
 * @param in: srcAddr: shared memory address from where data needs to be copied from
 * @param in: in_size: size of data to be copied
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_read(void *read_buffer, const void *srcAddr, const ssize_t &in_size)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(read_buffer ==NULL || in_size <= 0)
    {
        IVC_LOG_CRITICAL("Invalid read buffer or read size\n");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return false;
    }
    if(srcAddr)
    {
        memcpy(read_buffer, srcAddr, in_size);
        return true;
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return false;
}

/****************************************************
 * Write data to shared memory
 *
 * @param out: write_buffer: buffer from which data needs to be copied
 * @param in: destAddr: shared memory address to which data needs to be copied.
 * @param in: in_size: size of data to be copied
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_write(const void *write_buffer, void *destAddr, const ssize_t &in_size)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    if(write_buffer == NULL || in_size <= 0)
    {
        IVC_LOG_ERROR("Invalid write buffer or write size\n");
        IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
        return false;
    }

    if(destAddr)
    {
        memcpy(destAddr, write_buffer, in_size);
        return true;
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return false;
}


/****************************************************
 * Get Current VM Id.
 *
 * @param in -> devInstance: device instance
 * @param out -> vmid: virtual machine id
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_getVmId(const int32_t &devInstance, uint32_t &vmid)
{
    int32_t bar0fd = -1;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    bar0fd = open((char*)getBar0Dev(devInstance), O_RDWR);
    if(bar0fd <= 0)
    {
        IVC_LOG_ERROR("SHM open failed\n");
        return false;
    }
    else
    {
        uint32_t *barReg = NULL;
        barReg = (uint32_t *)(mmap(NULL, IVSH_BAR0_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, bar0fd, 0));
        if (barReg == MAP_FAILED) {
            IVC_LOG_ERROR("bar0_fd mmap failed\n");
            return false;
        }
        else
        {
            barReg += 2;
            vmid = *barReg;
        }
        IVC_LOG_DEBUG("VMID:%u\n", vmid);
        munmap(barReg, IVSH_BAR0_SIZE);
        close(bar0fd);
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


/****************************************************
 * Get Bar register access
 *
 * @param in ->  devInstance: memory instance
 * @param out ->  barReg: mapped bar register address
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_getRegAcess(const int32_t &devInstance, uint32_t *barReg)
{
    int32_t bar0fd = -1;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    bar0fd = open((char*)getBar0Dev(devInstance), O_RDWR);
    if(bar0fd <= 0)
    {
        IVC_LOG_ERROR("SHM open failed\n");
        return false;
    }
    else
    {
        barReg = (uint32_t *)(mmap(NULL, IVSH_BAR0_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, bar0fd, 0));
        if (barReg == MAP_FAILED) {
            IVC_LOG_ERROR("bar0_fd mmap failed\n");
            return false;
        }
    }
    //NOTE: This function only open and maps the registers. Once done, it should be unmapped and closed.

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


/****************************************************
 * Notify client attached to shared memory
 *
 * @param in ->  devInstance: memory instance
 * @param in ->  clientId: id of client to be notified
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_notifyClient(const int32_t &devInstance, const int32_t &clientId)
{
    int32_t bar0fd = -1;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

    bar0fd = open((char*)getBar0Dev(devInstance), O_RDWR);
    if(bar0fd <= 0)
    {
        IVC_LOG_ERROR("bar0_fd open failed\n");
        IVC_LOG_ERROR("SHM  open failed\n");
        return false;
    }
    else
    {
        uint32_t *p_reg = (uint32_t *)mmap(NULL, IVSH_BAR0_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, bar0fd, 0);
        if (p_reg == MAP_FAILED) {
            IVC_LOG_ERROR("bar0_fd mmap failed\n");
            close(bar0fd);
            return false;
        }
        IVC_LOG_DEBUG("lgivc_shmem_hal_private_notifyClients client:%d\n", clientId);
        p_reg[IVSH_REG_DOORBELL >> 2] = (clientId << 16) | IVSH_TEST_VECTOR_ID;
        munmap(p_reg, IVSH_BAR0_SIZE);
    }
    close(bar0fd);

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


/****************************************************
 * Wait for interrupt to be recived from other clients attached to shard memory
 *
 * @param in -> evtfd: event fd
 * @param in -> epfd: epoll fd
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_waitForData(const int32_t &evtfd, const int32_t &epfd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    struct epoll_event ev = {0};
    eventfd_t val;
    int32_t n;

    if(evtfd < 0 || epfd < 0)
    {
        IVC_LOG_ERROR("invalid evtfd or epfd value\n");
        return false;
    }
    n = epoll_wait(epfd, &ev, 1, 100);
    if(n == 0)
    {
        //IVC_LOG_DEBUG("epoll timeout\n");
        return false;
    }
    else if (n == 1) {
        eventfd_read(evtfd, &val);
        IVC_LOG_DEBUG("\t%sreturned with %ld, fd=%d\n", __func__, val, evtfd);
    }
    else if (n < 0 && errno != EINTR)
    {
        IVC_LOG_ERROR("epoll wait error %s\n", strerror(errno));
        return false;
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}



/****************************************************
 * Detach from shared memory
 *
 * @param in ->  memAddr: shared memory address
 * @param in ->  shmem_size: size of shared memory
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_detach(void **memAddr, const ssize_t &shmem_size)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    if(*memAddr)
    {
        munmap(*memAddr, shmem_size);
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


/****************************************************
 * Close the shared memory instance
 *
 * @param in ->  devfd: device fd
 * @return true on SUCCESS and false on FAILURE.
 *
 ****************************************************/
bool lgivc_shmem_hal_private_close(const int32_t &devFd)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    if(devFd <= 0)
    {
        IVC_LOG_ERROR("invalid devInstance value\n");
        return false;
    }

    close(devFd);

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}

