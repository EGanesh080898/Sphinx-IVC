#include <dirent.h>
#include <cstdint>
#include <cstring>
#include <cstddef>

#include "ivcLog.hpp"
#include "lgivc_shmem_hal_priv.hpp"

#define MAX_RESERVED_SHARED_MEMORY       15

static int8_t* getConfigDevice(int32_t devInstance)
{
    static int8_t cfgDev[100] = {0};

    sprintf((char*)cfgDev, SHMEM_CONFIG_DEVICE , devInstance);

    return cfgDev;
}


static int8_t* getBar2Cache(int32_t devInstance)
{
    static int8_t barDev[100] = {0};

    sprintf((char*)barDev, SHMEM_BAR2_CACHE_WC_DEVICE , devInstance);

    return barDev;
}


static bool getShmemSize(int32_t devInstance, uint32_t *shm_size)
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
    *shm_size = (size & ~(size - 1));
    close(configfd);

    return true;
}


static bool getNumberOfSharedMemory(uint32_t *sharedMemoryNum)
{
    uint32_t memCnt = 0;
    DIR *dirp = NULL;
    struct dirent *dp =NULL;

    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

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
    *sharedMemoryNum = memCnt;
    if(!memCnt)
    {
        *sharedMemoryNum = MAX_RESERVED_SHARED_MEMORY;
        IVC_LOG_WARNING("Setting Hardcoded number of shared memories = %d\n", memCnt);
    }
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


static bool shmem_memory_attach(int32_t devInstance, uint32_t memorySize, void **memAddr, int32_t flags, int32_t prot)
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
            IVC_LOG_ERROR("bar2_fd mmap failed\n");
            return false;
        }
    }
    IVC_LOG_DEBUG("%s mmap addr=%p\n", __FUNCTION__, *memAddr);

    IVC_LOG_DEBUG("fd = %d memorySize = %d\n", devInstance, memorySize);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);

    return true;
}

bool shmem_memory_detach(void **memAddr, int64_t shmem_size)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    if(*memAddr)
    {
        munmap(*memAddr, shmem_size);
    }

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return true;
}


int main()
{
    void     *memAddr;
    int32_t  flags = O_RDWR;
    int32_t  prot = PROT_READ | PROT_WRITE;
    uint32_t shmemSize = 0;
    uint32_t shMemNo = 0;
    lgivc_shmem_hal_meta *meta = NULL;

    setIvcLogLevel(IVC_LOG_INFO);

    getNumberOfSharedMemory(&shMemNo);

    //Loop through all the shared memories and see if it matches the memory name and size
    for(uint32_t i=0; i<shMemNo; i++)
    {
        if((true == getShmemSize(i, &shmemSize)) && (shmemSize >= 0))
        {
            memAddr = NULL;
            if(true == shmem_memory_attach(i, shmemSize, &memAddr, flags, prot) && memAddr)
            {
                meta = (lgivc_shmem_hal_meta *)memAddr;
                memset(meta, 0x0, sizeof(lgivc_shmem_hal_meta));
                IVC_LOG_INFO("Cleared memory @ %p with id=%d\n", meta, i);
                shmem_memory_detach(&memAddr, shmemSize);
            }
            else
            {
                IVC_LOG_ERROR("Failed to attach memory=%d & addr=%p\n", i, memAddr);
            }
        }
    }

    return 0;
}
