#ifndef POSIX_PTHREAD_H_
#define POSIX_PTHREAD_H_

/**
 * FreeRTOS Include
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>
#include <errno.h>
#include "time.h"
#include "sched.h"

#define PTHREAD_STACK_MIN          (configMINIMAL_STACK_SIZE * sizeof(BaseType_t))

#define PTHREAD_CREATE_DETACHED    0
#define PTHREAD_CREATE_JOINABLE    1

#define PTHREAD_EXPLICIT_SCHED      0
#define PTHREAD_INHERIT_SCHED       1

/* This is returned by pthread_barrier_wait.  It must not match any errno
 * in errno.h
 */
#define PTHREAD_BARRIER_SERIAL_THREAD 0x1000

/* Values for the mutext type attribute:
 *
 * PTHREAD_MUTEX_NORMAL: This type of mutex does not detect deadlock. A thread
 *   attempting to relock this mutex without first unlocking it will deadlock.
 *   Attempting to unlock a mutex locked by a different thread results in
 *   undefined behavior. Attempting to unlock an unlocked mutex results in
 *   undefined behavior.
 * PTHREAD_MUTEX_ERRORCHECK
 *   This type of mutex provides error checking. A thread attempting to relock
 *   this mutex without first unlocking it will return with an error. A thread
 *   attempting to unlock a mutex which another thread has locked will return
 *   with an error.   A thread attempting to unlock an unlocked mutex will return
 *   with an error.
 * PTHREAD_MUTEX_RECURSIVE
 *   A thread attempting to relock this mutex without first unlocking it will
 *   succeed in locking the mutex. The relocking deadlock which can occur with
 *   mutexes of type PTHREAD_MUTEX_NORMAL cannot occur with this type of mutex.
 *   Multiple locks of this mutex require the same number of unlocks to release
 *   the mutex before another thread can acquire the mutex. A thread attempting
 *   to unlock a mutex which another thread has locked will return with an error.
 *   A thread attempting to unlock an unlocked mutex will return with an error.
 * PTHREAD_MUTEX_DEFAULT
 *  An implementation is allowed to map this mutex to one of the other mutex
 *  types.
 */
#define PTHREAD_MUTEX_NORMAL          (0)
#define PTHREAD_MUTEX_ERRORCHECK      (1)
#define PTHREAD_MUTEX_RECURSIVE       (2)
#define PTHREAD_MUTEX_DEFAULT         PTHREAD_MUTEX_NORMAL

typedef void* pthread_t;
typedef void* pthread_barrierattr_t;
#if 0
typedef void* pthread_condattr_t;
#else
typedef struct {
    int      is_initialized;
    clock_t  clock;           /* specifiy clock for timeouts */
} pthread_condattr_t;
#endif

typedef struct __pthread_attr
{
    void* stackaddr;        /* stack address of thread */
    int   stacksize;        /* stack size of thread */

    int   inheritsched;     /* Inherit parent prio/policy */
    int   schedpolicy;      /* scheduler policy */
    struct sched_param schedparam; /* sched parameter */

    int   detachstate;      /* detach state */
} pthread_attr_t;

typedef struct __pthread_barrier
{
    unsigned cnt;
    unsigned threshold;
    StaticSemaphore_t sem_cnt;
    StaticEventGroup_t eventgrp;
} pthread_barrier_t;

typedef struct __pthread_cond
{
    BaseType_t initialized;
    StaticSemaphore_t wait_sem;
    unsigned wait_threads;
} pthread_cond_t;

typedef struct __pthread_mutexattr
{
    int type;
} pthread_mutexattr_t;

typedef struct __pthread_mutex
{
    BaseType_t initialized;
    StaticSemaphore_t mutex;
    TaskHandle_t owner;
    pthread_mutexattr_t attr;
} pthread_mutex_t;

#define PTHREAD_COND_INITIALIZER \
    (((pthread_cond_t)           \
    {                            \
        .initialized = pdFALSE,  \
        .wait_sem = {{0}},       \
        .wait_threads = 0        \
    }))

#define PTHREAD_MUTEX_INITIALIZER \
    (((pthread_mutex_t)           \
    {                             \
        .initialized = pdFALSE,   \
        .mutex = {{0}},           \
        .owner = NULL,            \
        .attr = {.type = 0}      \
    }))

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int state);
int pthread_attr_getdetachstate(pthread_attr_t const *attr, int *state);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getschedpolicy(pthread_attr_t const *attr, int *policy);
int pthread_attr_setschedparam(pthread_attr_t *attr,struct sched_param const *param);
int pthread_attr_getschedparam(pthread_attr_t const *attr,struct sched_param *param);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stack_size);
int pthread_attr_getstacksize(pthread_attr_t const *attr, size_t *stack_size);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack_addr);
int pthread_attr_getstackaddr(pthread_attr_t const *attr, void **stack_addr);
int pthread_attr_setstack(pthread_attr_t *attr, void *stack_base, size_t stack_size);
int pthread_attr_getstack(pthread_attr_t const *attr, void **stack_base, size_t *stack_size);
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guard_size);
int pthread_attr_getguardsize(pthread_attr_t const *attr, size_t *guard_size);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);
int pthread_attr_setscope(pthread_attr_t *attr, int scope);
int pthread_attr_getscope(pthread_attr_t const *attr, int *scope);

int pthread_create (pthread_t *tid, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
int pthread_detach (pthread_t thread);
int pthread_join (pthread_t thread, void **value_ptr);
void pthread_exit (void *value_ptr);
inline int pthread_equal (pthread_t t1, pthread_t t2) { return t1 == t2; }
pthread_t pthread_self (void);
int pthread_cancel (pthread_t thread);
int pthread_kill (pthread_t thread, int sig);
int pthread_setname_np (pthread_t thread, const char *name);
int pthread_getname_np (pthread_t thread, char *name, int namelen);
int pthread_getschedparam (pthread_t thread, int *policy, struct sched_param *param);
int pthread_setschedparam (pthread_t thread, int policy, const struct sched_param *param);

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_wait(pthread_barrier_t *barrier);

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id);
int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling);
int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int  pshared);
int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared);
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling);
int pthread_mutexattr_setprioceiling(const pthread_mutexattr_t *attr, int prioceiling);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol);
int pthread_mutexattr_setprotocol(const pthread_mutexattr_t *attr, int protocol);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
