#include <time.h>
#include <errno.h>

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    (void)clock_id;
    (void)tp;
    errno = EPERM;
    return -1;
}
