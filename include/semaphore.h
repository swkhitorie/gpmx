#ifndef POSIX_SEMAPHORE_H_
#define POSIX_SEMAPHORE_H_

#include <stdint.h>
#include "time.h"

/* FreeRTOS interface include */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

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

int        sem_init(FAR sem_t *sem, int pshared, unsigned int value);
int        sem_destroy(FAR sem_t *sem);
int        sem_wait(FAR sem_t *sem);
int        sem_timedwait(FAR sem_t *sem, FAR const struct timespec *abstime);
int        sem_trywait(FAR sem_t *sem);
int        sem_post(FAR sem_t *sem);
int        sem_getvalue(FAR sem_t *sem, FAR int *sval);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
