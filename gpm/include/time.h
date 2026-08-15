#ifndef __INCLUDE_TIME_H
#define __INCLUDE_TIME_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include "FreeRTOS.h"
#elif defined(CONFIG_RTTNANO_ENABLE)
#include "rtconfig.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_FREERTOS_ENABLE)
#define CLOCKS_PER_SEC     (configTICK_RATE_HZ)
#define MSEC_PER_TICK      (1000 / CLOCKS_PER_SEC)
#define USEC_PER_MSEC      (1000)
#define USEC_PER_TICK      ((USEC_PER_MSEC) * (MSEC_PER_TICK))
#elif defined(CONFIG_RTTNANO_ENABLE)
#define CLOCKS_PER_SEC     (RT_TICK_PER_SECOND)
#define MSEC_PER_TICK      (1000 / CLOCKS_PER_SEC)
#define USEC_PER_MSEC      (1000)
#define USEC_PER_TICK      ((USEC_PER_MSEC) * (MSEC_PER_TICK))
#else
#define CLOCKS_PER_SEC     1000
#define MSEC_PER_TICK      1
#define USEC_PER_MSEC      (1000)
#define USEC_PER_TICK      ((USEC_PER_MSEC) * (MSEC_PER_TICK))
#endif

#define CLOCK_REALTIME     0     // timestamp of unix 1970.1.1
#define CLOCK_MONOTONIC    1     // timestamp on boot
#define CLOCK_BOOTTIME     2

#define TIMER_ABSTIME      0x01

#define TIME_UTC           1

#define timelocal                 mktime
#define strftime_l(s, m, f, t, l) strftime(s, m, f, t)

#define MICROSECONDS_PER_SECOND    (1000000LL)
#define NANOSECONDS_PER_SECOND     (1000000000LL)
#define NANOSECONDS_PER_TICK       (NANOSECONDS_PER_SECOND/CLOCKS_PER_SEC)

/********************************************************************************
 * Public Type Definitions
 ********************************************************************************/

typedef uint32_t  time_t;         /* Holds time in seconds */
typedef uint32_t  clockid_t;      /* Identifies one time base source */
typedef void*     timer_t;        /* Represents one POSIX timer */

struct timespec
{
    time_t tv_sec;               /* Seconds */
    unsigned long tv_nsec;              /* Nanoseconds */
};

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
    long tm_gmtoff;      /* Offset from UTC in seconds */
    const char *tm_zone; /* Timezone abbreviation. */
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

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

clock_t clock(void);

int clock_settime(clockid_t clockid, const struct timespec *tp);
int clock_gettime(clockid_t clockid, struct timespec *tp);
int clock_getres(clockid_t clockid, struct timespec *res);
int timespec_get(struct timespec *t, int b);

time_t timegm(struct tm *tp);
time_t mktime(struct tm *tp);

struct tm *gmtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);

struct tm *localtime(const time_t *timep);
struct tm *localtime_r(const time_t *timep, struct tm *result);

size_t strftime(char *s, size_t max, const char *format,
                const struct tm *tm);
char *strptime(const char *s, const char *format,
                   struct tm *tm);

char *asctime(const struct tm *tp);
char *asctime_r(const struct tm *tp, char *buf);
char *ctime(const time_t *timep);
char *ctime_r(const time_t *timep, char *buf);

time_t time(time_t *timep);

#ifdef CONFIG_HAVE_DOUBLE
double difftime(time_t time1, time_t time0);
#else
float difftime(time_t time1, time_t time0);
#endif

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

#endif /* __INCLUDE_TIME_H */
