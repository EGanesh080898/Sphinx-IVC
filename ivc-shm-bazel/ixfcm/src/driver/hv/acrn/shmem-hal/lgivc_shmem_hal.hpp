#ifndef __LGIVC_SHMEM_HAL_H
#define __LGIVC_SHMEM_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <mutex>

typedef enum
{
    LGIVC_SHMEM_HAL_FAILURE,
    LGIVC_SHMEM_HAL_SUCCESS
}eLGIVC_SHMEM_HAL_RESULT;

typedef enum
{
    LGIVC_SHMEM_HAL_READ_ONLY,
    LGIVC_SHMEM_HAL_WRITE_ONLY,
    LGIVC_SHMEM_HAL_READ_WRITE,
}eLGIVC_SHMEM_HAL_MODES;


eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_open(const int32_t &devInstance, int32_t &bar0fd);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getNumberOfSharedMemory(uint32_t &sharedMemoryNum);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getNumberOfVMs(uint32_t &vmCount);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_registerIrq(int32_t &devfd, int32_t &evtfd, int32_t &epfd);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_deregisterIrq(const int32_t &evtfd, const int32_t &epfd);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getShmemSize(const int32_t &devInstance, ssize_t &shm_size);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_attach(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, void **memAddr, const eLGIVC_SHMEM_HAL_MODES &mode, int32_t &connId, uint32_t &vmId);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_attachToExistingMemory(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, void **memAddr, const eLGIVC_SHMEM_HAL_MODES &mode, int32_t &connId, uint32_t &vmId);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_attachToExistingMemory(const int32_t &devInstance, const std::string &memName, void **memAddr, const eLGIVC_SHMEM_HAL_MODES &mode);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_checkMemoryStatus(const int32_t &devInstance, const std::string &memName, const ssize_t &memorySize, bool &useStatus);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getDataAvailableForRead(void **memAddr, ssize_t &bytesAvailable);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_read(void *read_buffer, ssize_t in_size, void **memAddr, uint32_t vmId);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_read_from_offset(void *read_buffer, ssize_t in_size, void **memAddr, uint32_t vmId, int32_t offset);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_write(const void *write_buffer, const ssize_t &in_size, void **memAddr, const uint32_t &vmId);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_write_to_offset(const void *write_buffer, const ssize_t &in_size, void **memAddr, const uint32_t &vmId, int32_t offset);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getRegAcess(const int32_t &devInstance, uint32_t &barReg);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_notifyClient(const int32_t &devInstance, const int32_t &clientId);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_readStatus(void **memAddr, int32_t &status);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_clearMemory(void **memAddr);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_acknowledgeRead(const int32_t &devInstance, void **memAddr);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_waitForData(const int32_t &evtfd, const int32_t &epfd);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getAttachedVMs(void **memAddr, uint32_t &attachedVMs);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_detach(void **memAddr, const ssize_t &shmem_size);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_detach(void **memAddr);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getNumberOfClientsAttached(const void **memAddr, int32_t &clientsAttached);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_increment_clientCount(void **memAddr);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_decrement_clientCount(void **memAddr);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_setVMsInfo(void **memAddr, std::vector<uint32_t> &attachedVMs);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_getVMsInfo(void **memAddr, std::vector<uint32_t> &attachedVMs);
eLGIVC_SHMEM_HAL_RESULT lgivc_shmem_hal_close(const int32_t &fd);

#endif //__LGIVC_SHMEM_HAL_H
