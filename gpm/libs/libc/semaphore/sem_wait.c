#include <semaphore.h>
#include <errno.h>
#include <gpmx/config.h>

int sem_wait(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    if (!sem) {
        rt_set_errno(EINVAL);
        return -1;
    }

#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
    if (sem->protocol == SEM_PRIO_INHERIT) {
        result = rt_mutex_take(sem->mutex, RT_WAITING_FOREVER);
    } else
#endif
    {
        result = rt_sem_take(sem->sem, RT_WAITING_FOREVER);
    }

    if (result == RT_EOK) {
        return 0;
    }

    rt_set_errno(EINTR);
    return -EINTR;
#elif defined(CONFIG_FREERTOS_ENABLE)

    return sem_timedwait(sem, NULL);
#else

    return -1;
#endif
}

int sem_wait_uninterruptible(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    int ret;

    do {
        /* Take the semaphore (perhaps waiting) */
        ret = sem_wait(sem);
    } while (ret == -EINTR);

    return ret;
#elif defined(CONFIG_FREERTOS_ENABLE)

    return sem_wait(sem);
#else

    return -1;
#endif
}
