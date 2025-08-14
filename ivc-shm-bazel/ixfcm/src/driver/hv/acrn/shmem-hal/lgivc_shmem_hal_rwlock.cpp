/**************************************************************************************
 * lgivc_shmem_hal_rwlock.cpp
 *
 **************************************************************************************/
#include "lgivc_shmem_hal_rwlock.hpp"

/****************************************************
 *
 ****************************************************/
int32_t lgivc_shmem_hal_rwlock_init(ivcrwlock_t *rwlock)
{
    int32_t ret = 0;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);

#if 0
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
    pthread_rwlock_init(rwlock, &attr);
#endif
    ret = pthread_rwlock_init(rwlock, NULL);

    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return ret;
 }
 

/****************************************************
 *
 ****************************************************/
int32_t lgivc_shmem_hal_rwlock_deinit(ivcrwlock_t *rwlock)
{
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return 0;
}
 
/****************************************************
 *
 ****************************************************/
int32_t lgivc_shmem_hal_rwlock_readlock(ivcrwlock_t *rwlock)
{
    int32_t ret = 0;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    ret = pthread_rwlock_rdlock(rwlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return ret;
}
 
 
/****************************************************
 *
 ****************************************************/
int32_t lgivc_shmem_hal_rwlock_readunlock(ivcrwlock_t *rwlock)
{
    int32_t ret = 0;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    ret = pthread_rwlock_unlock(rwlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return ret;
}

/****************************************************
 *
 ****************************************************/
int32_t lgivc_shmem_hal_rwlock_writelock(ivcrwlock_t *rwlock)
{
    int32_t ret = 0;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    ret = pthread_rwlock_wrlock(rwlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return ret;
}
 
 
/****************************************************
 *
 ****************************************************/
int32_t lgivc_shmem_hal_rwlock_writeunlock(ivcrwlock_t *rwlock)
{
    int32_t ret = 0;
    IVC_LOG_ENTEXT("%s Enter\n",__FUNCTION__);
    ret = pthread_rwlock_unlock(rwlock);
    IVC_LOG_ENTEXT("%s Exit\n",__FUNCTION__);
    return ret;
}
