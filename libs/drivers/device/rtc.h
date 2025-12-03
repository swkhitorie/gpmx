#ifndef DEV_RTC_H_
#define DEV_RTC_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if !defined(CONFIG_RTC_TIME_LONGLONG)

/** UTC Timestamp from 1970/01/01 00:00:00 Thursday */
typedef uint32_t rclk_time_t;
#else

typedef uint64_t rclk_time_t;
#endif

struct rclk_tm {
    int _sec;   /* seconds after the minute, 0 to 60
                     (0 - 60 allows for the occasional leap second) */
    int _min;   /* minutes after the hour, 0 to 59 */
    int _hour;  /* hours since midnight, 0 to 23 */
    int _mday;  /* day of the month, 1 to 31 */
    int _mon;   /* months since January, 0 to 11 */
    int _year;  /* years since 1900 */
    int _wday;  /* days since Sunday, 0 to 6 */
};

struct rclk_timeval {
    rclk_time_t _sec;
    int _usec;
};

#if defined(__cplusplus)
extern "C"{
#endif

bool rclk_tm_to_timstamp(const struct rclk_tm *tm, rclk_time_t *timestamp);
bool rclk_timstamp_to_tm(rclk_time_t timestamp, struct rclk_tm *tm);

#if defined(__cplusplus)
}
#endif

#endif
