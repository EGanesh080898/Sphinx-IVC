#ifndef __LGIVC_MEMORYOBJECT_H
#define __LGIVC_MEMORYOBJECT_H

#include <string>
#include "lgivc_shmem_hal.hpp"
#include "ivcLog.hpp"

typedef struct
{
    std::string  memName;
    void         *shMemory;
    ssize_t      memSize;
    int32_t      connectionId;
    int32_t      devInstance;
    int32_t      clientFd;
    int32_t      eventFd;
    int32_t      epollFd;
    int32_t*     interruptReg;
    uint32_t     virtualMachineId;
}instanceData;


class cMemoryObj
{
        // friend class declaration
        friend class cMemoryManager;

        instanceData memory;

        /**************************************************************
        * write to shared Memory
        * 
        * @param in: buffer: buffer from which data needs to be copied
        * @param in: in_size: size of data to be copied
        * 
        * @return true on SUCCESS and false on FAILURE.
        **************************************************************/
        bool copy_to_memory(const void *buffer, const ssize_t &size)
        {
            if(buffer && (size <= memory.memSize))
            {
                return (lgivc_shmem_hal_write(buffer, size, &memory.shMemory, memory.virtualMachineId) == LGIVC_SHMEM_HAL_SUCCESS) ? true : false;
            }
            else
            {
                IVC_LOG_ERROR("%s failed: write size=%ld exceeds the sharedmemory size =%ld (buffer=%p)\n", __func__, size, memory.memSize, buffer);
            }
            return false;
        }

        /**************************************************************
        * write to shared Memory
        * 
        * @param in: buffer: buffer from which data needs to be copied
        * @param in: in_size: size of data to be copied
        * 
        * @return true on SUCCESS and false on FAILURE.
        **************************************************************/
        bool copy_to_memory(const void *buffer, const ssize_t &size, int32_t offset)
        {
            if(buffer && (size+offset <= memory.memSize))
            {
                return (lgivc_shmem_hal_write_to_offset(buffer, size, &memory.shMemory, memory.virtualMachineId, offset) == LGIVC_SHMEM_HAL_SUCCESS) ? true : false;
            }
            else
            {
                IVC_LOG_ERROR("%s failed: write size+offset=%ld exceeds the sharedmemory size =%ld (buffer=%p)\n", __func__, size+offset, memory.memSize, buffer);
            }
            return false;
        }

        /**************************************************************
        * read from shared Memory
        * 
        * @param out: buffer: buffer to which data needs to be copied
        * @param in: in_size: size of data to be copied
        * @return true on SUCCESS and false on FAILURE.
        **************************************************************/
        bool copy_from_memory(void *buffer, const ssize_t &size)
        {
            if(buffer && (size <= memory.memSize))
            {
                return (lgivc_shmem_hal_read(buffer, size, &memory.shMemory, memory.virtualMachineId) == LGIVC_SHMEM_HAL_SUCCESS) ? true : false;
            }
            else
            {
                IVC_LOG_ERROR("%s failed: read size=%ld exceeds the sharedmemory size =%ld (buffer=%p)\n", __func__, size, memory.memSize, buffer);
            }
            return false;
        }


        /**************************************************************
        * read from shared Memory
        * 
        * @param out: buffer: buffer to which data needs to be copied
        * @param in: in_size: size of data to be copied
        * @return true on SUCCESS and false on FAILURE.
        **************************************************************/
        bool copy_from_memory(void *buffer, const ssize_t &size, int32_t offset)
        {
            if(buffer && (size+offset <= memory.memSize))
            {
                return (lgivc_shmem_hal_read_from_offset(buffer, size, &memory.shMemory, memory.virtualMachineId, offset) == LGIVC_SHMEM_HAL_SUCCESS) ? true : false;
            }
            else
            {
                IVC_LOG_ERROR("%s failed: read size+offset=%ld exceeds the sharedmemory size =%ld (buffer=%p)\n", __func__, size+offset, memory.memSize, buffer);
            }
            return false;
        }

        /**************************************************************
        * get shared memory name
        * 
        * @param out: shmemName: shared memory name
        * @return true on SUCCESS and false on FAILURE.
        **************************************************************/
        bool get_sharedmemory_name(std::string &shmemName)
        {
            shmemName = memory.memName;
            return true;
        }

        /**************************************************************
        * get shared memory size
        * 
        * @param out: shmemSize: shared memory size
        * @return true on SUCCESS and false on FAILURE.
        **************************************************************/
        bool get_sharedmemory_size(ssize_t &shmemSize)
        {
            shmemSize = memory.memSize;
            return true;
        }

        /**************************************************************
        * get connection id
        *
        * @param out: shmemName: shared memory size
        * @return true on SUCCESS and false on FAILURE.
        **************************************************************/
        bool get_sharedmemory_connectionId(int32_t &connId)
        {
            connId = memory.connectionId;
            return true;
        }
    public:
        cMemoryObj()
        {
            memory.clientFd = -1;
            memory.eventFd = -1;
            memory.epollFd = -1;
            memory.interruptReg = nullptr;
            memory.memName = "";
            memory.shMemory = nullptr;
            memory.memSize = -1;
            memory.connectionId = -1;
            memory.virtualMachineId = -1;
        }
};

#endif // __LGIVC_MEMORYOBJECT_H
