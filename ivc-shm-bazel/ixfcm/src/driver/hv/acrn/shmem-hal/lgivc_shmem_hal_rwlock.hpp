
#ifndef __LGIVC_SHMEM_HAL_RWLOCK_H
#define __LGIVC_SHMEM_HAL_RWLOCK_H

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include "ivcLog.hpp"

typedef pthread_rwlock_t  ivcrwlock_t;

int32_t lgivc_shmem_hal_rwlock_init(ivcrwlock_t *rwlock);
int32_t lgivc_shmem_hal_rwlock_deinit(ivcrwlock_t *rwlock);
int32_t lgivc_shmem_hal_rwlock_readlock(ivcrwlock_t *rwlock);
int32_t lgivc_shmem_hal_rwlock_readunlock(ivcrwlock_t *rwlock);
int32_t lgivc_shmem_hal_rwlock_writelock(ivcrwlock_t *rwlock);
int32_t lgivc_shmem_hal_rwlock_writeunlock(ivcrwlock_t *rwlock);

#endif // __LGIVC_SHMEM_HAL_RWLOCK_H
