#include <stddef.h>
#include "pthread.h"
#include "errno.h"
#include "time.h"
#include "utils.h"

#include "./prv_timer.h"

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
    TimerHandle_t handle = timerid;
    timer_internal_t *p = (timer_internal_t *)pvTimerGetTimerID(handle);
    TickType_t next_expiration = xTimerGetExpiryTime(handle) - xTaskGetTickCount(),
                expiration_period = p->period;

    if (xTimerIsTimerActive(handle) != pdFALSE) {
        value->it_value.tv_sec = (time_t)(next_expiration / configTICK_RATE_HZ);
        value->it_value.tv_nsec = (long)((next_expiration % configTICK_RATE_HZ) * NANOSECONDS_PER_TICK);
    } else {
        value->it_value.tv_sec = 0;
        value->it_value.tv_nsec = 0;
    }

    if (expiration_period != portMAX_DELAY) {
        value->it_interval.tv_sec = (time_t)(expiration_period / configTICK_RATE_HZ);
        value->it_interval.tv_nsec = (long)((expiration_period % configTICK_RATE_HZ) * NANOSECONDS_PER_TICK);
    } else {
        value->it_interval.tv_sec = 0;
        value->it_interval.tv_nsec = 0;
    }
    return 0;
}
