
#ifndef POSIX_UTILS_H_
#define POSIX_UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include "time.h"

#include <FreeRTOS.h>

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

size_t  utils_strlen          (const char *p, size_t maxlen);
bool    utils_validtimespec   (const struct timespec *p);

int  utils_timespec_compare             (const struct timespec *x, const struct timespec *y);
int  utils_timespec_add                 (const struct timespec *x, const struct timespec *y, struct timespec *z);
int  utils_timespec_addnanoseconds      (const struct timespec *x, int64_t nanosec, struct timespec *y);
int  utils_timespec_subtract            (const struct timespec *x, const struct timespec *y, struct timespec *z);
int  utils_timespec_toticks             (const struct timespec *x, TickType_t *res);
void utils_nanoseconds_totimespec       (int64_t source, struct timespec *dst);
int  utils_timespec_todeltaticks        (const struct timespec *ab, const struct timespec *cur, TickType_t *res);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
