#include <semaphore.h>
#include <gpmx/config.h>

int sem_setprotocol(sem_t *sem, int protocol)
{
#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)

#if defined(CONFIG_RTTNANO_ENABLE)
    if (!sem || (protocol != SEM_PRIO_NONE && protocol != SEM_PRIO_INHERIT)) {
        rt_set_errno(EINVAL);
        return -1;
    }

    if (sem->protocol == protocol) {
        return 0;
    }

    /* Check if it is in a switchable state */
    if (sem->protocol == SEM_PRIO_INHERIT) {
        if (sem->mutex->owner != RT_NULL || sem->mutex->value != 1) {
            rt_set_errno(EBUSY);
            return -1;
        }
        rt_mutex_delete(sem->mutex);
    } else {
        if (rt_list_len(&sem->sem->parent.suspend_thread) != 0) {
            rt_set_errno(EBUSY);
            return -1;
        }
        rt_sem_delete(sem->sem);
    }

    if (protocol == SEM_PRIO_INHERIT) {
        sem->mutex = rt_mutex_create(sem->sem->parent.parent.name, RT_IPC_FLAG_FIFO);
        if (!sem->mutex) { 
            rt_set_errno(ENOMEM);
            return -1;
        }
        sem->sem = RT_NULL;
    } else {
        sem->sem = rt_sem_create(sem->mutex->parent.parent.name, 1, RT_IPC_FLAG_FIFO);
        if (!sem->sem) {
            rt_set_errno(ENOMEM);
            return -1;
        }
        sem->mutex = RT_NULL;
    }

    sem->protocol = protocol;

    return 0;
#elif defined(CONFIG_FREERTOS_ENABLE)

    if (!sem || (protocol != SEM_PRIO_NONE && protocol != SEM_PRIO_INHERIT)) {
        errno = EINVAL;
        return -1;
    }

    if (sem->protocol == protocol) {
        return 0;
    }

    if (sem->protocol == SEM_PRIO_INHERIT) {
        if (xSemaphoreGetMutexHolder(SEM_GET_HANDLE(sem)) != NULL ||
            uxSemaphoreGetCount(SEM_GET_HANDLE(sem)) == 0)
        {
            errno = EBUSY;
            return -1;
        }
    } else {
        if (uxSemaphoreGetCount(SEM_GET_HANDLE(sem)) != (UBaseType_t)sem->val) {
            errno = EBUSY;
            return -1;
        }
    }

#if defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
    vSemaphoreDelete(sem->handle);
    sem->handle = NULL;
#else
    memset(&sem->handle, 0, sizeof(StaticSemaphore_t));
#endif

    if (protocol == SEM_PRIO_INHERIT) {
#if defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
        sem->handle = xSemaphoreCreateMutex();
#else
        xSemaphoreCreateMutexStatic(&sem->handle);
#endif
        sem->val = 1;
    } else {
#if defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
        sem->handle = xSemaphoreCreateCounting(SEM_VALUE_MAX, sem->val);
#else
    #if defined(CONFIG_LIBC_SEMAPHORE_CACHE)
        xSemaphoreCreateCountingStatic(SEM_VALUE_MAX, 0, &sem->handle);
    #else
        xSemaphoreCreateCountingStatic(SEM_VALUE_MAX, sem->val, &sem->handle);
    #endif
#endif
    }

#if defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
    if (!sem->handle) { 
        errno = ENOMEM;
        return -1;
    }
#endif

    sem->protocol = protocol;
    return 0;
#else

    return -1;
#endif

#else // (CONFIG_LIBC_SEMAPHORE_INHERIT)
    return 0;
#endif
}
