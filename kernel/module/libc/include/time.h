#ifndef POSIX_TIME_H_
#define POSIX_TIME_H_

#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>

typedef uint32_t  time_t;         /* Holds time in seconds */
typedef uint32_t  clockid_t;      /* Identifies one time base source */
typedef void*     timer_t;        /* Represents one POSIX timer */

#if defined(CONFIG_FREERTOS_ENABLE)
#include "FreeRTOS.h"
#define CLOCKS_PER_SEC     ((clock_t)configTICK_RATE_HZ)
#elif defined(CONFIG_RTTNANO_ENABLE)
#include "rtdef.h"
#define CLOCKS_PER_SEC     ((clock_t)RT_TICK_PER_SECOND)
#else
#define CLOCKS_PER_SEC     1000
#endif

#define CLOCK_REALTIME     0
#define CLOCK_MONOTONIC    1
#define TIMER_ABSTIME      0x01

#define MICROSECONDS_PER_SECOND    (1000000LL)
#define NANOSECONDS_PER_SECOND     (1000000000LL)
#define NANOSECONDS_PER_TICK       (NANOSECONDS_PER_SECOND/CLOCKS_PER_SEC)

struct tm {
    int tm_sec;     /* Seconds (0-61, allows for leap seconds) */
    int tm_min;     /* Minutes (0-59) */
    int tm_hour;    /* Hours (0-23) */
    int tm_mday;    /* Day of the month (1-31) */
    int tm_mon;     /* Month (0-11) */
    int tm_year;    /* Years since 1900 */
    /* extended */
    int tm_wday;    /* Day of the week (0-6) */
    int tm_yday;    /* Day of the year (0-365) */
    int tm_isdst;   /* Non-0 if daylight savings time is in effect */
};

struct timespec
{
    time_t tv_sec;               /* Seconds */
    unsigned long tv_nsec;              /* Nanoseconds */
};

struct itimerspec
{
    struct timespec it_value;    /* First time */
    struct timespec it_interval; /* and thereafter */
};

#define TIMESPEC_IS_ZERO(obj)        (obj.tv_sec == 0 && obj.tv_nsec == 0)
#define TIMESPEC_IS_NOT_ZERO(obj)    (!(TIMESPEC_IS_ZERO(obj))) 

/* forward reference (defined in signal.h) */
struct sigevent;

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

clock_t clock(void);
double	difftime(time_t time2, time_t time1);

int clock_settime(clockid_t clockid,  const struct timespec *tp);
int clock_gettime(clockid_t clockid,  struct timespec *tp);
int clock_getres(clockid_t clockid,  struct timespec *res);

time_t mktime(struct tm *tp);
struct tm *gmtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);

int timer_create(clockid_t clockid,  struct sigevent *evp,  timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_settime(timer_t timerid, int flags,
                   const struct itimerspec *value,
                   struct itimerspec *ovalue);
int timer_gettime(timer_t timerid,  struct itimerspec *value);
int timer_getoverrun(timer_t timerid);

int clock_nanosleep(clockid_t clockid, int flags,
                     const struct timespec *rqtp,
                     struct timespec *rmtp);
int nanosleep( const struct timespec *rqtp,  struct timespec *rmtp);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
