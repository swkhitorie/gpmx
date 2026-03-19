#include <semaphore.h>
#include <errno.h>

int sem_trywait(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    if (!sem) {
        rt_set_errno(EINVAL);
        return -1;
    }

    result = rt_sem_take(sem->sem, 0);
    if (result == -RT_ETIMEOUT) {
        rt_set_errno(EAGAIN);
        return -1;
    }
    if (result == RT_EOK) {
        return 0;
    }

    rt_set_errno(EINTR);
    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    struct timespec xTimeout = {0};
    ret = sem_timedwait(sem, &xTimeout);
    if ((ret == -1 ) && errno == ETIMEDOUT) {
        errno = EAGAIN;
    }
    return ret;
#else

    return -1;
#endif
}
