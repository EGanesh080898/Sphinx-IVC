
#ifndef __LGIVC_SHMEM_HAL_PVT_H
#define __LGIVC_SHMEM_HAL_PVT_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <atomic>
#include "ivcLog.hpp"
#include "lgivc_shmem_hal_rwlock.hpp"


//Local Defines
#define SHMEM_DEVICE                     "/dev/uio%d"
#define SHMEM_CONFIG_DEVICE              "/sys/class/uio/uio%d/device/config"
#define SHMEM_BAR0_DEVICE                "/sys/class/uio/uio%d/device/resource0"
#define SHMEM_BAR2_CACHE_WC_DEVICE       "/sys/class/uio/uio%d/device/resource2_wc"
#define SHMEM_BAR2_CACHE_WB_DEVICE       "/sys/class/uio/uio%d/device/resource2_wb"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)

#define MAX_SHM_NAME_SIZE          100
#define MEMORY_IN_USE_SIGNATURE    0xA5A5A5A5
#define MAX_RESERVED_SHARED_MEMORY 15
#define SERVICE_VM_COUNT           1
#define USER_VM_COUNT              8
#define TOTAL_VM_COUNT             SERVICE_VM_COUNT+USER_VM_COUNT

//ACRN defines
#define UIO_IRQ_DATA _IOW('u', 100, struct uio_irq_data)
#define IVSH_TEST_VECTOR_ID        0U
#define IVSH_MAX_IRQ_NUM           8
#define IVSH_BAR0_SIZE             256
#define IVSH_REG_IVPOSITION        0x08
#define IVSH_REG_DOORBELL          0x0C



//Shared Memory Structure
typedef struct
{
    ivcrwlock_t               rwlock;
    int8_t                    shMemName[MAX_SHM_NAME_SIZE];
    uint32_t                  memInUse;
    ssize_t                   shMemSize;
    uint64_t                  attachedVMs;
    uint64_t                  writerVM;
    uint32_t                  clientId;
    uint32_t                  intendedClients;
    std::atomic<uint32_t>     attachCount;
    std::atomic<ssize_t>      dataSize;
    std::atomic<uint32_t>     readAckCount;
    std::atomic<uint32_t>     clientCount;
    std::atomic<bool>         dirty;
}__attribute__((packed, aligned(8))) lgivc_shmem_hal_meta;

struct uio_irq_data
{
    int32_t fd;
    int32_t vector;
};


bool lgivc_shmem_hal_private_open(const int32_t &devInstance, int32_t &devfd);
bool lgivc_shmem_hal_private_registerIrq(int32_t &devfd, int32_t &evtfd, int32_t &epfd);
bool lgivc_shmem_hal_private_deregisterIrq(const int32_t &evtfd, const int32_t &epfd);
bool lgivc_shmem_hal_private_getShmemSize(const int32_t &devInstance, uint32_t &shm_size);
bool lgivc_shmem_hal_private_attach(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, void **memAddr, const int32_t &flags, const int32_t &prot);
bool lgivc_shmem_hal_private_read(void *read_buffer, const void *srcAddr, const ssize_t &in_size);
bool lgivc_shmem_hal_private_write(const void *write_buffer, void *destAddr, const ssize_t &in_size);
bool lgivc_shmem_hal_private_getRegAcess(const int32_t &devInstance, uint32_t *barReg);
bool lgivc_shmem_hal_private_getVmId(const int32_t &devInstance, uint32_t &vmId);
bool lgivc_shmem_hal_private_notifyClient(const int32_t &devInstance, const int32_t &clientId);
bool lgivc_shmem_hal_private_waitForData(const int32_t &evtfd, const int32_t &epfd);
bool lgivc_shmem_hal_private_detach(void **memAddr, const ssize_t &shmem_size);
bool lgivc_shmem_hal_private_close(const int32_t &fd);

#endif
