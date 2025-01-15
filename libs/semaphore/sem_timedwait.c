#include "errno.h"
#include "semaphore.h"
#include "utils.h"
#include "atomic.h"

int sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
    int ret = 0;
    sem_t *p = (sem_t *)sem;
    TickType_t delay = portMAX_DELAY;

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

    int pre_val = Atomic_Decrement_u32((uint32_t *)&p->val);
    if (pre_val > 0) {
        ret = 0;
    } else {
        if (xSemaphoreTake((SemaphoreHandle_t) &p->sem, delay) != pdTRUE) {
            if( ret == 0 ) {
                // errno = ETIMEDOUT;
            } else {
                // errno = iStatus;
            }
            ret = -1;
        } else {
            ret = 0;
        }
    }
    return ret;
}
