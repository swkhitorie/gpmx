#include "errno.h"
#include "time.h"

/* FreeRTOS interface include */
#include <FreeRTOS.h>
#include <task.h>

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    TimeOut_t cur = { 0 };
    uint64_t tick_cnt = 0ULL;
    (void) clock_id;

    vTaskSetTimeOutState(&cur);
    tick_cnt = (uint64_t)(cur.xOverflowCount) << (sizeof(TickType_t) * 8);
    tick_cnt += cur.xTimeOnEntering;

    utils_nanoseconds_totimespec((int64_t)tick_cnt * NANOSECONDS_PER_TICK, tp);
    return 0;
}
