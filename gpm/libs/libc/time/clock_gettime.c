
#include <gpmx/config.h>
#include <time.h>
#include "utils.h"

#include <board_config.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#elif defined(CONFIG_RTTNANO_ENABLE)
#include <rtdef.h>
#include <rtthread.h>
#endif

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    TimeOut_t cur = {0};
#endif
    uint64_t tick_cnt = 0ULL;
    struct rtc_timeval now;

    switch (clock_id) {
    case CLOCK_MONOTONIC:
#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskSetTimeOutState(&cur);
        tick_cnt = (uint64_t)(cur.xOverflowCount) << (sizeof(TickType_t) * 8);
        tick_cnt += cur.xTimeOnEntering;

        utils_nanoseconds_totimespec((int64_t)tick_cnt * NANOSECONDS_PER_TICK, tp);
        return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)
        tick_cnt = rt_tick_get();
        utils_nanoseconds_totimespec((int64_t)tick_cnt * NANOSECONDS_PER_TICK, tp);
        return 0;
#else
        return -1;
#endif
        break;
    case CLOCK_REALTIME:
        tp->tv_sec = board_rtc_get_timestamp(&now);
        tp->tv_nsec = now._usec*1000;
        return 0;
    }

    return -1;
}
