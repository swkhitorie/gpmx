#ifndef __INCLUDE_SEMAPHORE_H
#define __INCLUDE_SEMAPHORE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

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

/************************************************************************
 * Pre-processor Definitions
 ************************************************************************/

#define SEM_PRIO_NONE             0
#define SEM_PRIO_INHERIT          1
#define SEM_PRIO_PROTECT          2

#define SEM_VALUE_MAX        0x7FFFU

/********************************************************************************
 * Public Type Definitions
 ********************************************************************************/

#if defined(CONFIG_FREERTOS_ENABLE)

#if defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
#define SEM_GET_HANDLE(sem)   ((sem)->handle)
#else
#define SEM_GET_HANDLE(sem)   ((SemaphoreHandle_t)&(sem)->handle)
#endif

typedef struct __sem_t 
{
#if !defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
    StaticSemaphore_t handle;
#else
    /* TODO: macro CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC will crash the system */
    SemaphoreHandle_t handle;
#endif
    int protocol;
    int val;
} sem_t;
#elif defined(CONFIG_RTTNANO_ENABLE)

struct posix_sem {
    rt_uint16_t refcount;
    rt_uint8_t unlinked;
    rt_uint8_t unamed;

    rt_sem_t sem;
#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
    rt_mutex_t  mutex;
    int         protocol;
#endif

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

/********************************************************************************
 * Public Function Prototypes
 ********************************************************************************/

int sem_close(sem_t *sem);
int sem_destroy(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_init(sem_t *sem, int pshared, unsigned int value);
sem_t *sem_open(const char *name, int oflag, ...);
int sem_post(sem_t *sem);
int sem_tickwait(sem_t *sem, uint32_t delay);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);
int sem_trywait(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_wait_uninterruptible(sem_t *sem);
int sem_unlink(const char *name);
int sem_setprotocol(sem_t *sem, int protocol);
int sem_getprotocol(sem_t *sem, int protocol);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_SEMAPHORE_H */
