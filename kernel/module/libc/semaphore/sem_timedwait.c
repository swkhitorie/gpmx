#include <semaphore.h>
#include <errno.h>
#include "utils.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include "atomic.h"
#endif

int sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    rt_int32_t tick;

    if (!sem || !abstime) {
        return EINVAL;
    }

    tick = rt_timespec_to_tick(abstime);
    result = rt_sem_take(sem->sem, tick);
    if (result == -RT_ETIMEOUT) {
        rt_set_errno(ETIMEDOUT);
        return -1;
    }
    if (result == RT_EOK) {
        return 0;
    }

    rt_set_errno(EINTR);
    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    sem_t *p = (sem_t *)sem;
    TickType_t delay = portMAX_DELAY;
    int pre_val = Atomic_Decrement_u32((uint32_t *)&p->val);

    if (abstime != NULL) {
        if (utils_validtimespec(abstime) == false) {
            delay = 0;
            ret = EINVAL;
        } else {
            struct timespec cur = {0};
            if (clock_gettime(CLOCK_REALTIME, &cur) != 0) {
                ret = EINVAL;
            } else {
                ret = utils_timespec_todeltaticks(abstime, &cur, &delay);
            }
            if (ret == ETIMEDOUT) {
                delay = 0;
            }
        }
    }

    if (pre_val > 0) {
        ret = 0;
    } else {
        if (xSemaphoreTake((SemaphoreHandle_t) &p->sem, delay) != pdTRUE) {
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
