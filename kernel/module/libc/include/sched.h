#ifndef POSIX_SCHED_H_
#define POSIX_SCHED_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/* POSIX-like scheduling policies */

#define SCHED_NORMAL              0  /* Alias to SCHED_OTHER */
#define SCHED_OTHER               0  /* Map to SCHED_FIFO or SCHED_RR */
#define SCHED_FIFO                1  /* FIFO priority scheduling policy */
#define SCHED_RR                  2  /* Round robin scheduling policy */
#define SCHED_SPORADIC            3  /* Sporadic scheduling policy */
#define SCHED_BATCH               4  /* Batch scheduling policy */
#define SCHED_IDLE                5  /* Idle scheduling policy */

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
void sched_start(void);

int sched_yield(void);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_setscheduler(pid_t pid, int policy);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
