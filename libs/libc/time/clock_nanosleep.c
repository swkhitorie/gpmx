#include "errno.h"
#include "time.h"
#include "utils.h"

/* FreeRTOS interface include */
#include <task.h>

int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp)
{
    int ret = 0;
    TickType_t sleep_time = 0;
    struct timespec cur = { 0 };

    (void)clock_id;
    (void)rmtp;
    (void)flags;

    if (utils_validtimespec( rqtp ) == false) {
        ret = EINVAL;
    }

    if (ret == 0 && clock_gettime(CLOCK_REALTIME, &cur) != 0) {
        ret = EINVAL;
    }

    if (ret == 0) {
        if ((flags & TIMER_ABSTIME) == TIMER_ABSTIME) {
            /* Get current time */
            if (clock_gettime(CLOCK_REALTIME, &cur) != 0) {
                ret = EINVAL;
            }

            if (ret == 0 && utils_timespec_todeltaticks(rqtp, &cur, &sleep_time) == 0) {
                #if (INCLUDE_vTaskDelayUntil == 1)
                    TickType_t cur_ticks = xTaskGetTickCount();
                    vTaskDelayUntil(&cur_ticks, sleep_time);
                #else
                    vTaskDelay( sleep_time );
                #endif
            }
        } else {
            if (utils_timespec_toticks(rqtp, &sleep_time) == 0) {
                vTaskDelay(sleep_time);
            }
        }
    }
    return ret;
}
