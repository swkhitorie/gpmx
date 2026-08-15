#ifndef INCLUDE_GPM_POSIX_SCHED_H
#define INCLUDE_GPM_POSIX_SCHED_H

#if defined(CONFIG_POSIXRUN_ENABLE)
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
static pthread_mutex_t posix_critical_mutex;
static int posix_critical_inited = 0;

static inline void posix_critical_init(void)
{
    if (!posix_critical_inited) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&posix_critical_mutex, &attr);
        posix_critical_inited = 1;
    }
}

static inline uint32_t posix_enter_critical(void)
{
    posix_critical_init();
    pthread_mutex_lock(&posix_critical_mutex);
    return 0;
}

static inline void posix_leave_critical(uint32_t flags)
{
    (void)flags;
    pthread_mutex_unlock(&posix_critical_mutex);
}

#define kmm_malloc(p)                       malloc(p)
#define kmm_free(p)                         free(p)
#define enter_critical_section()            posix_enter_critical()
#define leave_critical_section(x)           posix_leave_critical(x)
#define up_interrupt_context()              (0)

static inline uint64_t posix_gettime_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#define up_kerneltime()                     posix_gettime_ms()
#define kinterrupt_enter()                  do { } while (0)
#define kinterrupt_leave()                  do { } while (0)
#endif

#endif
