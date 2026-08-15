#ifndef DEV_RTC_H_
#define DEV_RTC_H_

#include <gpmx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if !defined(CONFIG_RTC_TIME_LONGLONG)

/** UTC Timestamp from 1970/01/01 00:00:00 Thursday */
typedef uint32_t rtc_time_t;
#else

typedef uint64_t rtc_time_t;
#endif

struct rtc_tm {
    int _sec;   /* seconds after the minute, 0 to 60
                     (0 - 60 allows for the occasional leap second) */
    int _min;   /* minutes after the hour, 0 to 59 */
    int _hour;  /* hours since midnight, 0 to 23 */
    int _mday;  /* day of the month, 1 to 31 */
    int _mon;   /* months since January, 0 to 11 */
    int _year;  /* years since 1900 */
    int _wday;  /* days since Sunday, 0 to 6 */
};

struct rtc_timeval {
    rtc_time_t _sec;
    int _usec;
};

#if defined(__cplusplus)
extern "C"{
#endif

bool rtc_tm_to_timstamp(const struct rtc_tm *tm, rtc_time_t *timestamp);
bool rtc_timstamp_to_tm(rtc_time_t timestamp, struct rtc_tm *tm);

#if defined(__cplusplus)
}
#endif

#endif
