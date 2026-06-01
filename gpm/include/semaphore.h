#ifndef POSIX_SEMAPHORE_H_
#define POSIX_SEMAPHORE_H_

#include <stdint.h>
#include <time.h>

#if defined(CONFIG_FREERTOS_ENABLE)

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#elif defined(CONFIG_RTTNANO_ENABLE)

#include <rthw.h>
#include <rtthread.h>
#endif

#define SEM_PRIO_NONE             0
#define SEM_PRIO_INHERIT          1
#define SEM_PRIO_PROTECT          2

#define SEM_VALUE_MAX        0x7FFFU

#if defined(CONFIG_FREERTOS_ENABLE)

typedef struct __sem_t 
{
    StaticSemaphore_t sem;
    int val;
} sem_t;
#elif defined(CONFIG_RTTNANO_ENABLE)

struct posix_sem {
    rt_uint16_t refcount;
    rt_uint8_t unlinked;
    rt_uint8_t unamed;

    rt_sem_t sem;

    struct posix_sem* next;
};
typedef struct posix_sem sem_t;
#else

typedef int sem_t;
#endif

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int sem_close(sem_t *sem);
int sem_destroy(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_init(sem_t *sem, int pshared, unsigned int value);
sem_t *sem_open(const char *name, int oflag, ...);
int sem_post(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);
int sem_trywait(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_unlink(const char *name);
int sem_setprotocol(sem_t *sem, int protocol);
int sem_getprotocol(sem_t *sem, int protocol);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
