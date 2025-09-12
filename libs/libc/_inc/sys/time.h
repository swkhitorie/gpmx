#ifndef __INCLUDE_SYS_TIME_H
#define __INCLUDE_SYS_TIME_H

#include <time.h>

#define ITIMER_REAL    0 /* Timers run in real time. */
#define ITIMER_VIRTUAL 1 /* Timers run only when the process is executing. */
#define ITIMER_PROF    2 /* Timers run when the process is executing and when
                          * the system is executing on behalf of the process.
                          */

/* The following are non-standard interfaces in the sense that they are not
 * in POSIX.1-2001 nor are they specified at OpenGroup.org. These interfaces
 * are present on most BSD derivatives, however, including Linux.
 */

/* void timeradd(FAR struct timeval *a, FAR struct timeval *b,
 *               FAR struct timeval *res);
 */

#define timeradd(tvp, uvp, vvp) \
  do \
    { \
      (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec; \
      (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec; \
      if ((vvp)->tv_usec >= 1000000) \
        { \
          (vvp)->tv_sec++; \
          (vvp)->tv_usec -= 1000000; \
        } \
    } \
  while (0)

/* void timersub(FAR struct timeval *a, FAR struct timeval *b,
 *               FAR struct timeval *res);
 */

#define timersub(tvp, uvp, vvp) \
  do \
    { \
      (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
      (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
      if ((vvp)->tv_usec < 0) \
        { \
          (vvp)->tv_sec--; \
          (vvp)->tv_usec += 1000000; \
        } \
    } \
  while (0)

/* void timerclear(FAR struct timeval *tvp); */

#define timerclear(tvp) \
  do \
    { \
      (tvp)->tv_sec = 0; \
      (tvp)->tv_usec = 0; \
    } \
  while (0)

/* int timerisset(FAR struct timeval *tvp); */

#define timerisset(tvp) \
  ((tvp)->tv_sec != 0 || (tvp)->tv_usec != 0)

/* int timercmp(FAR struct timeval *a, FAR struct timeval *b, CMP); */

#define timercmp(tvp, uvp, cmp) \
  (((tvp)->tv_sec == (uvp)->tv_sec) ? \
   ((tvp)->tv_usec cmp (uvp)->tv_usec) : \
   ((tvp)->tv_sec cmp (uvp)->tv_sec))

/* Macros for converting between `struct timeval' and `struct timespec'.  */

#define TIMEVAL_TO_TIMESPEC(tv, ts) \
  do \
    { \
      (ts)->tv_sec = (tv)->tv_sec; \
      (ts)->tv_nsec = (tv)->tv_usec * 1000; \
    } \
  while (0)

#define TIMESPEC_TO_TIMEVAL(tv, ts) \
  do \
    { \
      (tv)->tv_sec = (ts)->tv_sec; \
      (tv)->tv_usec = (ts)->tv_nsec / 1000; \
    } \
  while (0)

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/* struct timeval represents time as seconds plus microseconds */

struct timeval
{
  time_t tv_sec;         /* Seconds */
  long tv_usec;          /* Microseconds */
};

/* Type of the second argument to `getitimer' and
 * the second and third arguments `setitimer'.
 */

struct itimerval
{
  struct timeval it_interval; /* Interval for periodic timer */
  struct timeval it_value;    /* Time until next expiration */
};

/* The use of the struct timezone  is obsolete; the tz argument should
 * normally be specified as NULL (and is ignored in any event).
 */

struct timezone
{
  int tz_minuteswest;     /* Minutes west of Greenwich */
  int tz_dsttime;         /* Type of DST correction */
};

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif



#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_SYS_TIME_H */
