#include <semaphore.h>
#include <gpmx/config.h>

int sem_getvalue(sem_t *sem, int *sval)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    if (!sem || !sval) {
        rt_set_errno(EINVAL);
        return -1;
    }

#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
    if (sem->protocol == SEM_PRIO_INHERIT) {
        /* 1: unlock, 0: lock */
        *sval = sem->mutex->value;
    } else
#endif
    {
        *sval = sem->sem->value;
    }

    return 0;
#elif defined(CONFIG_FREERTOS_ENABLE)

    if (!sem || !sval) { 
        errno = EINVAL;
        return -1;
    }

#if defined(CONFIG_LIBC_SEMAPHORE_CACHE)
    if (sem->protocol != SEM_PRIO_INHERIT) {
        *sval = sem->val;
    } else
#else
    {
        *sval = (int)uxSemaphoreGetCount(SEM_GET_HANDLE(sem));
    }

#endif

    return 0;

#else

    return -1;
#endif
}
