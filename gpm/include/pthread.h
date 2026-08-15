#ifndef __INCLUDE_PTHREAD_H
#define __INCLUDE_PTHREAD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <stdint.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>

#if defined(CONFIG_FREERTOS_ENABLE)

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>
#elif defined(CONFIG_RTTNANO_ENABLE)

#include <rthw.h>
#include <rtthread.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_FREERTOS_ENABLE)

#define CONFIG_PTHREAD_STACK_MIN          (configMINIMAL_STACK_SIZE * sizeof(BaseType_t))
#define CONFIG_PTHREAD_STACK_DEFAULT      (CONFIG_PTHREAD_STACK_MIN * 2)
#define CONFIG_PTHREAD_GUARDSIZE_DEFAULT  (CONFIG_PTHREAD_STACK_DEFAULT)
#define CONFIG_PTHREAD_COND_MAX_WAITERS   (4)
#elif defined(CONFIG_RTTNANO_ENABLE)

#define CONFIG_PTHREAD_STACK_MIN          (128 * 4)
#define CONFIG_PTHREAD_STACK_DEFAULT      (CONFIG_PTHREAD_STACK_MIN * 2)
#define CONFIG_PTHREAD_GUARDSIZE_DEFAULT  (CONFIG_PTHREAD_STACK_DEFAULT)
#define PTHREAD_KEY_MAX                   (8)
#endif

/* Values for the process shared (pshared) attribute */

#define PTHREAD_PROCESS_PRIVATE       0
#define PTHREAD_PROCESS_SHARED        1

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
#define PTHREAD_MUTEX_ERRORCHECK_NP   PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_RECURSIVE_NP    PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_DEFAULT         PTHREAD_MUTEX_NORMAL

/* Valid ranges for the pthread stacksize attribute */

#define PTHREAD_STACK_MIN             CONFIG_PTHREAD_STACK_MIN
#define PTHREAD_STACK_DEFAULT         CONFIG_PTHREAD_STACK_DEFAULT
#define PTHREAD_GUARD_DEFAULT         CONFIG_PTHREAD_GUARDSIZE_DEFAULT

/* Values for the pthread inheritsched attribute */

#define PTHREAD_INHERIT_SCHED         0
#define PTHREAD_EXPLICIT_SCHED        1

/* Detach state  */

#define PTHREAD_CREATE_JOINABLE       0
#define PTHREAD_CREATE_DETACHED       1

/* Cancellation states used by pthread_setcancelstate() */

#define PTHREAD_CANCEL_ENABLE         (0)
#define PTHREAD_CANCEL_DISABLE        (1)

/* Cancellation types used by pthread_setcanceltype() */

#define PTHREAD_CANCEL_DEFERRED       (0)
#define PTHREAD_CANCEL_ASYNCHRONOUS   (1)
#define PTHREAD_CANCELED              (2)

/* This is returned by pthread_barrier_wait.  It must not match any errno
 * in errno.h
 */
#define PTHREAD_BARRIER_SERIAL_THREAD 0x1000

/* Values for protocol mutex attribute */

#define PTHREAD_PRIO_NONE             SEM_PRIO_NONE
#define PTHREAD_PRIO_INHERIT          SEM_PRIO_INHERIT
#define PTHREAD_PRIO_PROTECT          SEM_PRIO_PROTECT

/* The contention scope attribute in thread attributes object */

#define PTHREAD_SCOPE_SYSTEM          0
#define PTHREAD_SCOPE_PROCESS         1

#if defined(CONFIG_FREERTOS_ENABLE)

#define PTHREAD_COND_INITIALIZER \
    (((pthread_cond_t)           \
    {                            \
        .initialized = -1,       \
        .wait_task = NULL,       \
        .wait_count = CONFIG_PTHREAD_COND_MAX_WAITERS \
    }))

#define PTHREAD_MUTEX_INITIALIZER \
    (((pthread_mutex_t)           \
    {                             \
        .initialized = -1,        \
        .lock = {{0}},            \
        .owner = NULL,            \
        .attr = -1                \
    }))
#elif defined(CONFIG_RTTNANO_ENABLE)

#define PTHREAD_COND_INITIALIZER \
    (((pthread_cond_t)           \
    {                            \
        .attr = -1,              \
    }))

#define PTHREAD_MUTEX_INITIALIZER \
    (((pthread_mutex_t)           \
    {                             \
        .attr = -1,               \
    }))

#endif

/********************************************************************************
 * Public Type Definitions
 ********************************************************************************/

#if defined(CONFIG_FREERTOS_ENABLE)
typedef void* pthread_t;
#elif defined(CONFIG_RTTNANO_ENABLE)
typedef long  pthread_t;
#endif

typedef struct {
    int pshared;
    clockid_t clockid;
} pthread_condattr_t;

typedef long pthread_rwlockattr_t;
typedef long pthread_mutexattr_t;      /* (pshared) PTHREAD_PROCESS_PRIVATE or PTHREAD_PROCESS_SHARED */
typedef long pthread_barrierattr_t;    /* (pshared) PTHREAD_PROCESS_PRIVATE or PTHREAD_PROCESS_SHARED */

typedef struct __pthread_attr
{
    void* stackaddr;        /* stack address of thread */
    int   stacksize;        /* stack size of thread */

    int   inheritsched;     /* Inherit parent prio/policy */
    int   schedpolicy;      /* scheduler policy */
    struct sched_param schedparam; /* sched parameter */

    int   detachstate;      /* detach state */
} pthread_attr_t;

typedef struct __pthread_mutex
{
    pthread_mutexattr_t attr;
#if defined(CONFIG_FREERTOS_ENABLE)
    StaticSemaphore_t lock;
    int initialized;
    TaskHandle_t owner;
#elif defined(CONFIG_RTTNANO_ENABLE)
    struct rt_mutex lock;
#endif
} pthread_mutex_t;

typedef struct __pthread_cond
{
    pthread_condattr_t attr;
#if defined(CONFIG_FREERTOS_ENABLE)
    TaskHandle_t *wait_task;
    int initialized;
    int wait_count;
#elif defined(CONFIG_RTTNANO_ENABLE)
    struct rt_semaphore sem;
#endif
} pthread_cond_t;

typedef struct pthread_rwlock {
    pthread_rwlockattr_t attr;

    pthread_mutex_t      rw_mutex;          /* basic lock on this struct */
    pthread_cond_t       rw_condreaders;    /* for reader threads waiting */
    pthread_cond_t       rw_condwriters;    /* for writer threads waiting */

    int rw_nwaitreaders;    /* the number of reader threads waiting */
    int rw_nwaitwriters;    /* the number of writer threads waiting */
    int rw_refcount;    /* 0: unlocked, -1: locked by writer, > 0 locked by n readers */
} pthread_rwlock_t;

/* spinlock implementation, (ADVANCED REALTIME THREADS)*/
typedef struct __pthread_spinlock
{
    int lock;
} pthread_spinlock_t;

typedef struct __pthread_barrier
{
    int count;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} pthread_barrier_t;

typedef int pthread_key_t;
#if 0
typedef struct {
    bool done;
    pthread_mutex_t mutex;
} pthread_once_t;
#else
typedef int pthread_once_t;
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
int pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
int pthread_setname_np(pthread_t thread, const char *name);
int pthread_getname_np(pthread_t thread, char *name, int namelen);
int pthread_detach(pthread_t thread);
int pthread_join(pthread_t thread, void **value_ptr);
static inline int pthread_equal(pthread_t t1, pthread_t t2) { return t1 == t2; }
pthread_t pthread_self(void);

int pthread_getcpuclockid(pthread_t thread, clockid_t *clock_id);
int pthread_getconcurrency(void);
int pthread_setconcurrency(int new_level);
int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
int pthread_setschedprio(pthread_t thread, int prio);

void pthread_exit(void *value_ptr);
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

/* pthread cleanup */
void pthread_cleanup_pop(int execute);
void pthread_cleanup_push(void (*routine)(void*), void *arg);

int pthread_cancel(pthread_t thread);
void pthread_testcancel(void);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
int pthread_kill(pthread_t thread, int sig);

/****************************************************************************
 * pthread mutex interface
 ****************************************************************************/
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

/****************************************************************************
 * pthread condition interface
 ****************************************************************************/
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id);
int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

/****************************************************************************
 * pthread rwlock interface
 ****************************************************************************/
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared);
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared);

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);

int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abstime);
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, const struct timespec *abstime);

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);

/****************************************************************************
 * pthread spinlock interface
 ****************************************************************************/
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);

int pthread_spin_lock(pthread_spinlock_t * lock);
int pthread_spin_trylock(pthread_spinlock_t * lock);
int pthread_spin_unlock(pthread_spinlock_t * lock);

/****************************************************************************
 * pthread barrier interface
 ****************************************************************************/
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
int pthread_barrierattr_init(pthread_barrierattr_t *attr);
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared);
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared);

int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count);
int pthread_barrier_wait(pthread_barrier_t *barrier);

void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);
int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
int pthread_key_delete(pthread_key_t key);

struct filelist;
struct streamlist;
struct filelist *pt_sched_get_files(void);
struct streamlist *pt_sched_get_streams(void);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_PTHREAD_H */
