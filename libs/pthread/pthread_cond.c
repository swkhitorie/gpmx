#include <stddef.h>
#include <string.h>
#include <limits.h>
#include "pthread.h"
#include "errno.h"
#include "atomic.h"


static void init_condstatic(pthread_cond_t *p)
{
    if (p->initialized == pdFALSE) {
        taskENTER_CRITICAL();

        if (p->initialized == pdFALSE) {
            p->initialized = pdTRUE;
            (void)xSemaphoreCreateCountingStatic(INT_MAX, 0U, &p->wait_sem);
            p->wait_threads = 0;
        }
        taskEXIT_CRITICAL();
    }
}

static void test_and_decrement(pthread_cond_t *p, unsigned local_waitThread)
{
    while( local_waitThread > 0 )
    {
        if( ATOMIC_COMPARE_AND_SWAP_SUCCESS == Atomic_CompareAndSwap_u32(
            (uint32_t *) &p->wait_threads, (uint32_t)local_waitThread - 1, (uint32_t)local_waitThread)) {
            break;
        }
        local_waitThread = p->wait_threads;
    }
}

int pthread_condattr_init(pthread_condattr_t *attr)
{
    (void)attr;
    return -1;
}

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    (void)attr;
    return -1;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id)
{
    (void)attr;
    (void)clock_id;
    return -1;
}

int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id)
{
    (void)attr;
    (void)clock_id;
    return -1;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    int ret = 0;
    pthread_cond_t *p = (pthread_cond_t *)cond;

    (void)attr;
    if (p == NULL) {
        ret = ENOMEM;
    }

    if (ret == 0) {
        p->initialized = pdTRUE;
        (void)xSemaphoreCreateCountingStatic(INT_MAX, 0U, &p->wait_sem);
        p->wait_threads = 0;
    }
    return ret;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    pthread_cond_t *p = (pthread_cond_t *)cond;
    vSemaphoreDelete((SemaphoreHandle_t) &p->wait_sem);
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    unsigned i = 0;
    pthread_cond_t *p = (pthread_cond_t *)cond;

    init_condstatic(p);

    unsigned local_waitThread = p->wait_threads;
    while (local_waitThread > 0) {
        if( ATOMIC_COMPARE_AND_SWAP_SUCCESS == Atomic_CompareAndSwap_u32(
            (uint32_t *)&p->wait_threads, 0, (uint32_t)local_waitThread)) {
            for (i = 0; i < local_waitThread; i++) {
                (void)xSemaphoreGive((SemaphoreHandle_t)&p->wait_sem);
            }
            break;
        }
        local_waitThread = p->wait_threads;
    }
    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    pthread_cond_t *p = (pthread_cond_t *)cond;
    init_condstatic(p);

    unsigned local_waitThread = p->wait_threads;
    while (local_waitThread > 0) {
        if( ATOMIC_COMPARE_AND_SWAP_SUCCESS == Atomic_CompareAndSwap_u32(
            (uint32_t *)&p->wait_threads, (uint32_t)local_waitThread - 1, (uint32_t)local_waitThread)) {
            (void)xSemaphoreGive((SemaphoreHandle_t)&p->wait_sem);
            break;
        }
        local_waitThread = p->wait_threads;
    }
    return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    unsigned local_waitThread;
    int ret = 0;
    pthread_cond_t *p = (pthread_cond_t *)cond;
    TickType_t delay = portMAX_DELAY;

    init_condstatic(p);
    if (abstime != NULL) {
        struct timespec cur = { .tv_nsec = 0, .tv_sec = 0, };
        if (clock_gettime(CLOCK_REALTIME, &cur) != 0) {
            ret = EINVAL;
        } else {
            ret = utils_timespec_todeltaticks(abstime, &cur, &delay);
        }
    }

    if (ret == 0) {
        local_waitThread = Atomic_Increment_u32((uint32_t *) &p->wait_threads);
        ret = pthread_mutex_unlock(mutex);
    }

    if (ret == 0) {
        if (xSemaphoreTake((SemaphoreHandle_t)&p->wait_sem, delay) == pdPASS) {
            ret = pthread_mutex_lock(mutex);
        } else {
            ret = ETIMEDOUT;
            (void)pthread_mutex_lock(mutex);
            test_and_decrement(p, local_waitThread + 1);
        }
    } else {
        test_and_decrement(p, local_waitThread + 1);
    }
    return ret;
}


int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    return pthread_cond_timedwait(cond, mutex, (const struct timespec *)NULL);
}
