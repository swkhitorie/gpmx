#ifndef POSIX_SCHED_H_
#define POSIX_SCHED_H_

#include <stdint.h>
#include <stdbool.h>
#include "sys/types.h"

#define SCHED_FIFO                1  /* FIFO priority scheduling policy */
#define SCHED_RR                  2  /* Round robin scheduling policy */
#define SCHED_SPORADIC            3  /* Sporadic scheduling policy */
#define SCHED_OTHER               4  /* Not supported */

#define SCHED_MIN       SCHED_OTHER
#define SCHED_MAX       SCHED_RR

struct sched_param
{
    int sched_priority;    /* Base thread priority */
};

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* start scheduler in freertos */
void   sched_start(void);

int    sched_yield(void);
int    sched_get_priority_max(int policy);
int    sched_get_priority_min(int policy);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
