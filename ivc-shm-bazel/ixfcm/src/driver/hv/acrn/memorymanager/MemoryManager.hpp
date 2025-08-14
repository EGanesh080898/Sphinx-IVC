#ifndef __LGIVC_MEMORYMANAGER_H
#define __LGIVC_MEMORYMANAGER_H

#include "ivcLog.hpp"
#include <vector>
#include <mutex>
#include <set>

typedef struct
{
    uint32_t uioDev;
    ssize_t  shmemSize;
    bool     inUse;
}memoryInfo;

struct sortInsert
{
    bool operator()(const std::shared_ptr<memoryInfo> &a, const std::shared_ptr<memoryInfo> &b) const
    {
        return a->shmemSize < b->shmemSize;
    }
};

typedef std::multiset<std::shared_ptr<memoryInfo>, sortInsert> sortedMap;

class cMemoryManager
{
    std::shared_ptr<void> memObj;
    static std::mutex mlock;

    static sortedMap memoryMap;

    int32_t populateSharedMemoryInfo(void);
    int32_t updateSharedMemoryInfo(void);
    int32_t getNextFreeMemory(uint32_t &uioDevNode, const ssize_t &reqMemSize);
    int32_t allocMemory(std::string memName, const ssize_t &reqMemSize);
    int32_t attachToExistingMemory(std::string memName, const ssize_t &reqMemSize);
    int32_t getClientList(std::vector<int32_t> &clientList);

public:
    cMemoryManager();
    ~cMemoryManager();

    int32_t getSharedMemory(std::string memName, const ssize_t &memSize);
    int32_t releaseSharedMemory(void);
    std::shared_ptr<void> getMemoryObject(void);
    int32_t copy_to_memory(const void *buffer, const ssize_t &size);
    int32_t copy_to_memory(const void *buffer, const ssize_t &size, int32_t offset);
    int32_t copy_from_memory(void *buffer, const ssize_t &size);
    int32_t copy_from_memory(void *buffer, const ssize_t &size, int32_t offset);
    int32_t wait_for_data(void);
    int32_t get_bytes_available(ssize_t &bytes);
    int32_t get_read_status(int32_t &status);
    int32_t clearMemory(void);
    int32_t get_shMemName(std::string &shMemName);
    int32_t get_shMemSize(ssize_t &shMemSize);
    int32_t get_shMemConnectionId(int32_t &connId);
    int32_t acknowledge_read(void);
    int32_t notifyClients(std::vector<int32_t> &clientList);
    int32_t notifyClients(void);
    int32_t getNumberOfClientsAttached(int32_t &clientsAttached);
};

#endif //__LGIVC_MEMORYMANAGER_H
