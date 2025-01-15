#ifndef POSIX_TIME_H_
#define POSIX_TIME_H_

#include "sys/types.h"
#include <stdbool.h>
#include <errno.h>

#include <FreeRTOS.h>

typedef uint32_t  time_t;         /* Holds time in seconds */
typedef uint8_t   clockid_t;      /* Identifies one time base source */
typedef FAR void* timer_t;        /* Represents one POSIX timer */

#define MICROSECONDS_PER_SECOND    (1000000LL)
#define NANOSECONDS_PER_SECOND     (1000000000LL)
#define NANOSECONDS_PER_TICK       (NANOSECONDS_PER_SECOND/configTICK_RATE_HZ)

#define CLOCK_REALTIME     0
#define CLOCK_MONOTONIC    1
#define CLOCKS_PER_SEC     ((clock_t)configTICK_RATE_HZ)
#define TIMER_ABSTIME      0x01

struct timespec
{
    time_t tv_sec;               /* Seconds */
    long   tv_nsec;              /* Nanoseconds */
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

int clock_settime(clockid_t clockid, FAR const struct timespec *tp);
int clock_gettime(clockid_t clockid, FAR struct timespec *tp);
int clock_getres(clockid_t clockid, FAR struct timespec *res);

int timer_create(clockid_t clockid, FAR struct sigevent *evp, FAR timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_settime(timer_t timerid, int flags,
                  FAR const struct itimerspec *value,
                  FAR struct itimerspec *ovalue);
int timer_gettime(timer_t timerid, FAR struct itimerspec *value);
int timer_getoverrun(timer_t timerid);

int clock_nanosleep(clockid_t clockid, int flags,
                    FAR const struct timespec *rqtp,
                    FAR struct timespec *rmtp);
int nanosleep(FAR const struct timespec *rqtp, FAR struct timespec *rmtp);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
