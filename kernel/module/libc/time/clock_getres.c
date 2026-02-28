#include "errno.h"
#include "time.h"

int clock_getres(clockid_t clock_id, struct timespec *res)
{
    (void)clock_id;

    if (res != NULL) {
        res->tv_sec = 0;
        res->tv_nsec = NANOSECONDS_PER_TICK;
    }
    return 0;
}
