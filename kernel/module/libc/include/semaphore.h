#ifndef POSIX_SEMAPHORE_H_
#define POSIX_SEMAPHORE_H_

#include <stdint.h>
#include "time.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#endif

#define SEM_PRIO_NONE             0
#define SEM_PRIO_INHERIT          1
#define SEM_PRIO_PROTECT          2

#define SEM_VALUE_MAX        0x7FFFU

typedef struct __sem_t 
{
    StaticSemaphore_t sem;
    int val;
} sem_t;

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int        sem_init(sem_t *sem, int pshared, unsigned int value);
int        sem_destroy(sem_t *sem);
int        sem_wait(sem_t *sem);
int        sem_timedwait(sem_t *sem,  const struct timespec *abstime);
int        sem_trywait(sem_t *sem);
int        sem_post(sem_t *sem);
int        sem_getvalue(sem_t *sem, int *sval);
int        sem_setprotocol(sem_t *sem, int protocol);
int        sem_getprotocol(sem_t *sem, int protocol);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
