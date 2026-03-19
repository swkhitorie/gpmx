
#include <pthread.h>
#include <errno.h>

#include "utils.h"

#define MUTEXATTR_SHARED_MASK 0x0010
#define MUTEXATTR_TYPE_MASK   0x000f

const pthread_mutexattr_t pthread_default_mutexattr = PTHREAD_PROCESS_PRIVATE;

#if defined(CONFIG_FREERTOS_ENABLE)

static void init_staticmutix(pthread_mutex_t *mutex)
{
    if (mutex->initialized == -1) {
        taskENTER_CRITICAL();
        if (mutex->initialized == -1) {
            mutex->attr = pthread_default_mutexattr;
            #if PTHREAD_MUTEX_DEFAULT == PTHREAD_MUTEX_RECURSIVE
                xSemaphoreCreateRecursiveMutexStatic(&mutex->lock);
            #else
                xSemaphoreCreateMutexStatic(&mutex->lock);
            #endif
            mutex->initialized = 0;
        }
        taskEXIT_CRITICAL();
    }
}
#endif

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (attr) {
        *attr = pthread_default_mutexattr;
        return 0;
    }

    return EINVAL;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (attr) {
        *attr = -1;
        return 0;
    }

    return EINVAL;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (attr && type) {
        int atype = (*attr & MUTEXATTR_TYPE_MASK);
        if (atype >= PTHREAD_MUTEX_NORMAL && atype <= PTHREAD_MUTEX_ERRORCHECK) {
            *type = atype;
            return 0;
        }
    }

    return EINVAL;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (attr && type >= PTHREAD_MUTEX_NORMAL && type <= PTHREAD_MUTEX_ERRORCHECK) {
        *attr = (*attr & ~MUTEXATTR_TYPE_MASK) | type;
        return 0;
    }

    return EINVAL;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
    if (!attr) {
        return EINVAL;
    }

    switch (pshared) {
    case PTHREAD_PROCESS_PRIVATE:
        *attr &= ~MUTEXATTR_SHARED_MASK;
        return 0;

    case PTHREAD_PROCESS_SHARED:
        *attr |= MUTEXATTR_SHARED_MASK;
        return 0;
    }

    return EINVAL;
}

int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared)
{
    if (!attr || !pshared) {
        return EINVAL;
    }

    *pshared = (*attr & MUTEXATTR_SHARED_MASK) ? PTHREAD_PROCESS_SHARED
                                            : PTHREAD_PROCESS_PRIVATE;
    return 0;
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

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    pthread_mutex_t *p = (pthread_mutex_t *)mutex;
    if (p == NULL) {
        ret = ENOMEM;
    }

    if (ret == 0) {
        *p = PTHREAD_MUTEX_INITIALIZER;
        p->attr = (attr == NULL) ? pthread_default_mutexattr : *attr;
        if ((p->attr & MUTEXATTR_TYPE_MASK) == PTHREAD_MUTEX_RECURSIVE) {
            xSemaphoreCreateRecursiveMutexStatic(&p->lock);
        } else {
            xSemaphoreCreateMutexStatic(&p->lock);
        }
        if ((SemaphoreHandle_t)&p->lock == NULL) {
            ret = EAGAIN;
            vPortFree(p);
        } else {
            p->initialized = 0;
        }
    }
    return ret;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    char name[RT_NAME_MAX];
    static rt_uint16_t pthread_mutex_number = 0;

    if (!mutex) {
        return EINVAL;
    }

    rt_snprintf(name, sizeof(name), "pmtx%02d", pthread_mutex_number++);
    if (attr == RT_NULL) {
        mutex->attr = pthread_default_mutexattr;
    } else {
        mutex->attr = *attr;
    }

    result = rt_mutex_init(&(mutex->lock), name, RT_IPC_FLAG_PRIO);
    if (result != RT_EOK) {
        return EINVAL;
    }

    /* detach the object from system object container */
    rt_object_detach(&(mutex->lock.parent.parent));
    mutex->lock.parent.parent.type = RT_Object_Class_Mutex;

    return 0;
#endif
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    pthread_mutex_t *p = (pthread_mutex_t *)mutex;
    if (p->owner == NULL) {
        vSemaphoreDelete((SemaphoreHandle_t)&p->lock);
    }
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    if (!mutex || mutex->attr == -1) {
        return EINVAL;
    }

    /* it's busy */
    if (mutex->lock.owner != RT_NULL) {
        return EBUSY;
    }

    rt_memset(mutex, 0, sizeof(pthread_mutex_t));
    mutex->attr = -1;
    return 0;
#endif
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return pthread_mutex_timedlock(mutex, NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)

    int mtype;
    rt_err_t result;

    if (!mutex) {
        return EINVAL;
    }

    if (mutex->attr == -1) {
        pthread_mutex_init(mutex, RT_NULL);
    }

    mtype = mutex->attr & MUTEXATTR_TYPE_MASK;
    rt_enter_critical();
    if (mutex->lock.owner == rt_thread_self() &&
        mtype != PTHREAD_MUTEX_RECURSIVE) {
        rt_exit_critical();
        return EDEADLK;
    }
    rt_exit_critical();

    result = rt_mutex_take(&(mutex->lock), RT_WAITING_FOREVER);
    if (result == RT_EOK) {
        return 0;
    }

    return EINVAL;
#endif

}

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    pthread_mutex_t *p = (pthread_mutex_t *)mutex;
    TickType_t delay = portMAX_DELAY;
    BaseType_t fr_mutex_take_status = pdFALSE;

    init_staticmutix(p);
    configASSERT(p->initialized == 0);

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

    if ((ret == 0) && ((p->attr & MUTEXATTR_TYPE_MASK) == PTHREAD_MUTEX_ERRORCHECK) && 
        (p->owner == xTaskGetCurrentTaskHandle())) {
        ret = EDEADLK;
    }

    if (ret == 0) {
        if ((p->attr & MUTEXATTR_TYPE_MASK) == PTHREAD_MUTEX_RECURSIVE) {
            fr_mutex_take_status = xSemaphoreTakeRecursive((SemaphoreHandle_t)&p->lock, delay);
        } else {
            fr_mutex_take_status = xSemaphoreTake((SemaphoreHandle_t)&p->lock, delay);
        }

        if (fr_mutex_take_status == pdPASS) {
            p->owner = xTaskGetCurrentTaskHandle();
        } else {
            ret = ETIMEDOUT;
        }
    }
    return ret;
#endif
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

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
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    int mtype;

    if (!mutex) {
        return EINVAL;
    }

    if (mutex->attr == -1) {
        pthread_mutex_init(mutex, RT_NULL);
    }

    mtype = mutex->attr & MUTEXATTR_TYPE_MASK;
    rt_enter_critical();
    if (mutex->lock.owner == rt_thread_self() &&
        mtype != PTHREAD_MUTEX_RECURSIVE) {
        rt_exit_critical();
        return EDEADLK;
    }
    rt_exit_critical();

    result = rt_mutex_take(&(mutex->lock), 0);
    if (result == RT_EOK) {
        return 0;
    }

    return EBUSY;
#endif
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    pthread_mutex_t *p = (pthread_mutex_t *)mutex;

    init_staticmutix(p);

    if((((p->attr & MUTEXATTR_TYPE_MASK) == PTHREAD_MUTEX_ERRORCHECK) ||
        ((p->attr & MUTEXATTR_TYPE_MASK) == PTHREAD_MUTEX_RECURSIVE)) &&
        (p->owner != xTaskGetCurrentTaskHandle())) {
        ret = EPERM;
    }

    if (ret == 0) {
        vTaskSuspendAll();
        if ((p->attr & MUTEXATTR_TYPE_MASK) == PTHREAD_MUTEX_RECURSIVE) {
            xSemaphoreGiveRecursive((SemaphoreHandle_t)&p->lock);
        } else {
            xSemaphoreGive((SemaphoreHandle_t)&p->lock);
        }
        p->owner = xSemaphoreGetMutexHolder((SemaphoreHandle_t)&p->lock);
        xTaskResumeAll();
    }
    return ret;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;

    if (!mutex) {
        return EINVAL;
    }

    if (mutex->attr == -1) {
        pthread_mutex_init(mutex, RT_NULL);
    }

    if (mutex->lock.owner != rt_thread_self()) {
        int mtype;
        mtype = mutex->attr & MUTEXATTR_TYPE_MASK;

        /* error check, return EPERM */
        if (mtype == PTHREAD_MUTEX_ERRORCHECK) {
            return EPERM;
        }

        /* no thread waiting on this mutex */
        if (mutex->lock.owner == RT_NULL) {
            return 0;
        }
    }

    result = rt_mutex_release(&(mutex->lock));
    if (result == RT_EOK) {
        return 0;
    }

    return EINVAL;
#endif
}

int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling)
{
    return pthread_mutexattr_getprioceiling(&mutex->attr, prioceiling);
}

int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling)
{
    *old_ceiling = pthread_mutexattr_getprioceiling(&mutex->attr, old_ceiling);
    if(*old_ceiling != 0) {
        return EINVAL;
    }

    return pthread_mutexattr_setprioceiling(&mutex->attr, prioceiling);
}
