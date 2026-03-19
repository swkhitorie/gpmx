#include <semaphore.h>
#include <errno.h>

int sem_wait(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    if (!sem) {
        rt_set_errno(EINVAL);
        return -1;
    }

    result = rt_sem_take(sem->sem, RT_WAITING_FOREVER);
    if (result == RT_EOK) {
        return 0;
    }

    rt_set_errno(EINTR);
    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    return sem_timedwait(sem, NULL);
#else

    return -1;
#endif
}
