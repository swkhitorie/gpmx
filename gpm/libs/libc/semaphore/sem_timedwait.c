#include <semaphore.h>
#include <errno.h>
#include "utils.h"
#include "prv_sem.h"
#include <gpmx/config.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include "atomic.h"
#endif

int sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    rt_int32_t tick = 0;

    if (!sem || !abstime) {
        return EINVAL;
    }

    if (abstime != NULL) {
        if (utils_validtimespec(abstime) == false) {
            tick = 0;
            rt_set_errno(EINVAL);
        } else {
            struct timespec cur = {0};
            if (clock_gettime(CLOCK_MONOTONIC, &cur) != 0) {
                rt_set_errno(EINVAL);
            } else {
                if (ETIMEDOUT == utils_timespec_todeltaticks(abstime, &cur, &tick)) {
                    tick = 0;
                }
            }
        }
    }

    // tick = rt_timespec_to_tick(abstime);
#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
    if (sem->protocol == SEM_PRIO_INHERIT) {
        result = rt_mutex_take(sem->mutex, tick);
    } else
#endif
    {
        result = rt_sem_take(sem->sem, tick);
    }

    if (result == -RT_ETIMEOUT) {
        rt_set_errno(ETIMEDOUT);
        return -1;
    }

    if (result == RT_EOK) {
        rt_set_errno(EINTR);
        return -1;
    }

    return 0;
#elif defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    TickType_t delay = portMAX_DELAY;
    int pre_val = 0;

    if (!sem) {
        errno = EINVAL;
        return -1;
    }

    if (abstime != NULL) {
        if (utils_validtimespec(abstime) == false) {
            delay = 0;
            ret = EINVAL;
        } else {
            struct timespec cur = {0};
            if (clock_gettime(CLOCK_MONOTONIC, &cur) != 0) {
                ret = EINVAL;
            } else {
                ret = utils_timespec_todeltaticks(abstime, &cur, &delay);
            }
            if (ret == ETIMEDOUT) {
                delay = 0;
            }
        }
    }

#if defined(CONFIG_LIBC_SEMAPHORE_CACHE)
    pre_val = Atomic_Decrement_u32((uint32_t *)&sem->val);
    if (pre_val > 0 && sem->protocol != SEM_PRIO_INHERIT) {
        ret = 0;
    } else
#endif
    {
        if (xSemaphoreTake(SEM_GET_HANDLE(sem), delay) != pdTRUE) {
            if (ret == 0) {
                errno = ETIMEDOUT;
            } else {
                errno = ret;
            }
            ret = -1;
        } else {
            ret = 0;
        }
    }
    return ret;

#else

    return -1;
#endif
}

int sem_tickwait(sem_t *sem, uint32_t delay)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    if (!sem) {
        rt_set_errno(EINVAL);
        return -1;
    }

    result = rt_sem_take(sem->sem, delay);
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
    int pre_val = 0;

#if defined(CONFIG_LIBC_SEMAPHORE_CACHE)
    pre_val = Atomic_Decrement_u32((uint32_t *)&sem->val);
    if (pre_val > 0) {
        ret = 0;
    } else
#endif
    {
        if (xSemaphoreTake(SEM_GET_HANDLE(sem), delay) != pdTRUE) {
            if (ret == 0) {
                errno = ETIMEDOUT;
            } else {
                errno = ret;
            }
            ret = -1;
        } else {
            ret = 0;
        }
    }
    return ret;

#else

    return -1;
#endif
}
