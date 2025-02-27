#include <stddef.h>
#include "pthread.h"
#include "errno.h"
#include "time.h"
#include "utils.h"

#include "./prv_timer.h"

int timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue)
{
    int ret = 0;
    TimerHandle_t handle = timerid;
    timer_internal_t *p = (timer_internal_t *)pvTimerGetTimerID(handle);
    TickType_t next_expiration = 0, expiration_period = 0;

    if (TIMESPEC_IS_NOT_ZERO(value->it_value)) {
        if((utils_validtimespec(&value->it_interval) == false) ||
        (utils_validtimespec(&value->it_value) == false)) {
            // errno = EINVAL;
            ret = -1;
        }
    }

    if (ovalue != NULL) {
        (void)timer_gettime(timerid, ovalue);
    }

    if ( ret == 0 && xTimerIsTimerActive(handle)) {
        (void)xTimerStop(handle, portMAX_DELAY);
    }

    if (ret == 0 && TIMESPEC_IS_NOT_ZERO(value->it_value)) {
        if (TIMESPEC_IS_NOT_ZERO(value->it_interval)) {
            (void)utils_timespec_toticks(&value->it_interval, &expiration_period);
        }

        p->period = expiration_period;
        if (TIMESPEC_IS_NOT_ZERO(value->it_value)) {
            if ((flags & TIMER_ABSTIME) == TIMER_ABSTIME) {
                struct timespec cur = { 0 };

                if (clock_gettime(CLOCK_REALTIME, &cur) != 0) {
                    ret = EINVAL;
                } else {
                    ret = utils_timespec_todeltaticks(&value->it_value, &cur, &next_expiration);
                }

                if (ret != 0) {
                    next_expiration = 0;
                    if (ret == ETIMEDOUT) {
                        ret = 0;
                    }
                }
            } else {
                (void)utils_timespec_toticks(&value->it_value, &next_expiration);
            }
        }

        if (next_expiration == 0) {
            prv_timer_callback(handle);
        } else {
            (void)xTimerChangePeriod(handle, next_expiration, portMAX_DELAY);
            (void)xTimerStart(handle, next_expiration);
        }
    }
    return ret;
}
