#include <semaphore.h>
#include <errno.h>
#include "prv_sem.h"
#include <gpmx/config.h>

int sem_init(sem_t *sem, int pshared, unsigned value)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    char name[RT_NAME_MAX];
    static rt_uint16_t psem_number = 0;
    (void)pshared;

    if (sem == RT_NULL) {
        rt_set_errno(EINVAL);
        return -1;
    }

    rt_snprintf(name, sizeof(name), "psem%02d", psem_number++);

#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
    sem->protocol = SEM_PRIO_INHERIT;
    sem->sem      = RT_NULL;
    sem->mutex = rt_mutex_create(name, RT_IPC_FLAG_FIFO);
    if (sem->mutex == RT_NULL) { 
        rt_set_errno(ENOMEM);
        return -1; 
    }
#else
    sem->sem = rt_sem_create(name, value, RT_IPC_FLAG_FIFO);
    if (sem->sem == RT_NULL) {
        rt_set_errno(ENOMEM);
        return -1;
    }
#endif

    /* initialize posix semaphore */
    sem->refcount = 1;
    sem->unlinked = 0;
    sem->unamed = 1;

    posix_sem_lock();
    posix_sem_insert(sem);
    posix_sem_unlock();
#elif defined(CONFIG_FREERTOS_ENABLE)

    (void)pshared;
    if (!sem || value > SEM_VALUE_MAX) { 
        errno = EINVAL;
        return -1;
    }

    sem->val = value;
    sem->protocol = SEM_PRIO_INHERIT;

#if !defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
    xSemaphoreCreateMutexStatic(&sem->handle);
#else
    sem->handle = xSemaphoreCreateMutex();
    if (sem->handle == NULL) { 
        errno = ENOMEM;
        return -1;
    }
#endif

    return 0;

#else

    return -1;
#endif
}

