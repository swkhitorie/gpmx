#include <stddef.h>
#include <string.h>
#include "pthread.h"
#include "errno.h"
#include "utils.h"

static void init_staticmutix(pthread_mutex_t *mutex)
{
    if (mutex->initialized == pdFALSE) {
        taskENTER_CRITICAL();
        if (mutex->initialized == pdFALSE) {
            mutex->attr.type = PTHREAD_MUTEX_DEFAULT;
            #if PTHREAD_MUTEX_DEFAULT == PTHREAD_MUTEX_RECURSIVE
                (void)xSemaphoreCreateRecursiveMutexStatic(&mutex->mutex);
            #else
                (void)xSemaphoreCreateMutexStatic(&mutex->mutex);
            #endif
            mutex->initialized = pdTRUE;
        }
        taskEXIT_CRITICAL();
    }
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    int ret = 0;
    pthread_mutex_t *p = (pthread_mutex_t *)mutex;
    pthread_mutexattr_t m_default = { .type = PTHREAD_MUTEX_NORMAL, };
    if (p == NULL) {
        ret = ENOMEM;
    }

    if (ret == 0) {
        *p = PTHREAD_MUTEX_INITIALIZER;
        p->attr = (attr == NULL) ? m_default : *attr;
        if (p->attr.type == PTHREAD_MUTEX_RECURSIVE) {
            (void)xSemaphoreCreateRecursiveMutexStatic(&p->mutex);
        } else {
            (void)xSemaphoreCreateMutexStatic(&p->mutex);
        }
        if ((SemaphoreHandle_t)&p->mutex == NULL) {
            ret = EAGAIN;
            vPortFree(p);
        } else {
            p->initialized = pdTRUE;
        }
    }
    return ret;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    pthread_mutex_t *p = (pthread_mutex_t *)mutex;
    if (p->owner == NULL) {
        vSemaphoreDelete((SemaphoreHandle_t)&p->mutex);
    }
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    return pthread_mutex_timedlock(mutex, NULL);
}

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
    int ret = 0;
    pthread_mutex_t *p = (pthread_mutex_t *)mutex;
    TickType_t delay = portMAX_DELAY;
    BaseType_t fr_mutex_take_status = pdFALSE;

    init_staticmutix(p);
    configASSERT(p->initialized == pdTRUE);

    if (abstime != NULL) {
        struct timespec cur = {0};
        if (clock_gettime(CLOCK_REALTIME, &cur)!= 0) {
            ret = EINVAL;
        } else {
            ret = utils_timespec_todeltaticks(abstime, &cur, &delay);
        }
        if (ret == ETIMEDOUT) {
            delay = 0;
            ret = 0;
        }
    }

    if ((ret == 0) && (p->attr.type == PTHREAD_MUTEX_ERRORCHECK) && 
        (p->owner == xTaskGetCurrentTaskHandle())) {
        ret = EDEADLK;
    }

    if (ret == 0) {
        if (p->attr.type == PTHREAD_MUTEX_RECURSIVE) {
            fr_mutex_take_status = xSemaphoreTakeRecursive((SemaphoreHandle_t)&p->mutex, delay);
        } else {
            fr_mutex_take_status = xSemaphoreTake((SemaphoreHandle_t)&p->mutex, delay);
        }

        if (fr_mutex_take_status == pdPASS) {
            p->owner = xTaskGetCurrentTaskHandle();
        } else {
            ret = ETIMEDOUT;
        }
    }
    return ret;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    int ret = 0;
    struct timespec timeout =
    {
        .tv_sec  = 0,
        .tv_nsec = 0
    };

    ret = pthread_mutex_timedlock(mutex, &timeout);
    if (ret == ETIMEDOUT) {
        ret = EBUSY;
    }
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    int ret = 0;
    pthread_mutex_t *p = (pthread_mutex_t *)mutex;

    init_staticmutix(p);

    if(((p->attr.type == PTHREAD_MUTEX_ERRORCHECK) ||
        (p->attr.type == PTHREAD_MUTEX_RECURSIVE)) &&
        (p->owner != xTaskGetCurrentTaskHandle())) {
        ret = EPERM;
    }

    if (ret == 0) {
        vTaskSuspendAll();
        if (p->attr.type == PTHREAD_MUTEX_RECURSIVE) {
            (void)xSemaphoreGiveRecursive((SemaphoreHandle_t)&p->mutex);
        } else {
            (void)xSemaphoreGive((SemaphoreHandle_t)&p->mutex);
        }
        p->owner = xSemaphoreGetMutexHolder((SemaphoreHandle_t)&p->mutex);
        (void)xTaskResumeAll();
    }
    return ret;
}

int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling)
{
    (void)mutex;
    (void)prioceiling;
    return -1;
}

int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling)
{
    (void)mutex;
    (void)prioceiling;
    (void)old_ceiling;
    return -1;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    pthread_mutexattr_t m_default = { .type = PTHREAD_MUTEX_NORMAL, };
    *attr = m_default;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    (void)attr;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    pthread_mutexattr_t *p = (pthread_mutexattr_t *)attr;
    *type = p->type;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    int ret = 0;
    pthread_mutexattr_t *p = (pthread_mutexattr_t *)attr;

    switch (type) {
        case PTHREAD_MUTEX_NORMAL:
        case PTHREAD_MUTEX_RECURSIVE:
        case PTHREAD_MUTEX_ERRORCHECK:
            p->type = type;
            break;
        default:
            ret = EINVAL;
            break;
    }
    return ret;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
    (void)attr;
    (void)pshared;
    return -1;
}

int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared)
{
    (void)attr;
    (void)pshared;
    return -1;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
    (void)attr;
    (void)prioceiling;
    return -1;
}

int pthread_mutexattr_setprioceiling(const pthread_mutexattr_t *attr, int prioceiling)
{
    (void)attr;
    (void)prioceiling;
    return -1;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
    (void)attr;
    (void)protocol;
    return -1;
}

int pthread_mutexattr_setprotocol(const pthread_mutexattr_t *attr, int protocol)
{
    (void)attr;
    (void)protocol;
    return -1;
}
