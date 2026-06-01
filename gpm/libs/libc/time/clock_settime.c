#include <time.h>
#include <errno.h>
#include <board_config.h>

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    bool result = false;
    switch (clock_id) {
    case CLOCK_REALTIME:
        result = board_rtc_set_timestamp(tp->tv_sec);
        break;
    }

    return result;
}
