#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#include "utils.h"

#if defined(CONFIG_FREERTOS_ENABLE)

static bool init_condstatic(pthread_cond_t *p)
{
    int i = 0;

    if (p->initialized == -1) {
        vTaskSuspendAll();
        if (p->initialized == -1 && p->wait_count > 0) {
            p->wait_task = pvPortMalloc(sizeof(TaskHandle_t) * p->wait_count);
            if (p->wait_task != NULL) {
                for (i = 0; i < p->wait_count; i++ ) {
                    p->wait_task[i] = NULL;
                }
                p->initialized = 0;
            }
        }
        xTaskResumeAll();
    }

    return (p->initialized == 0);
}
#elif defined(CONFIG_RTTNANO_ENABLE)

rt_err_t _pthread_cond_timedwait(pthread_cond_t *cond,
                                pthread_mutex_t *mutex,
                                rt_int32_t timeout)
{
    rt_err_t result = RT_EOK;
    rt_sem_t sem;
    rt_int32_t time;

    sem = &(cond->sem);
    if (sem == RT_NULL) {
        return -RT_ERROR;
    }

    time = timeout;

    if (!cond || !mutex) {
        return -RT_ERROR;
    }

    /* check whether initialized */
    if (cond->attr.pshared == -1) {
        pthread_cond_init(cond, RT_NULL);
    }

    /* The mutex was not owned by the current thread at the time of the call. */
    if (mutex->lock.owner != rt_thread_self()) {
        return -RT_ERROR;
    }

    {
        struct rt_thread *thread;

        /* parameter check */
        RT_ASSERT(sem != RT_NULL);
        RT_ASSERT(rt_object_get_type(&sem->parent.parent) == RT_Object_Class_Semaphore);

        rt_enter_critical();

        if (sem->value > 0) {
            /* semaphore is available */
            sem->value--;
            rt_exit_critical();
        } else {
            /* no waiting, return with timeout */
            if (time == 0) {
                rt_exit_critical();
                return -RT_ETIMEOUT;
            } else {
                /* current context checking */
                RT_DEBUG_IN_THREAD_CONTEXT;

                /* semaphore is unavailable, push to suspend list */
                /* get current thread */
                thread = rt_thread_self();

                /* reset thread error number */
                thread->error = RT_EOK;

                /* suspend thread */
                rt_thread_suspend(thread);

                /* Only support FIFO */
                rt_list_insert_before(&(sem->parent.suspend_thread), &RT_THREAD_LIST_NODE(thread));

                /**
                rt_ipc_list_suspend(&(sem->parent.suspend_thread),
                                    thread,
                                    sem->parent.parent.flag);
                */

                /* has waiting time, start thread timer */
                if (time > 0) {
                    /* reset the timeout of thread timer and start it */
                    rt_timer_control(&(thread->thread_timer),
                                    RT_TIMER_CTRL_SET_TIME,
                                    &time);
                    rt_timer_start(&(thread->thread_timer));
                }

                /* to avoid the lost of singal< cond->sem > */
                if (pthread_mutex_unlock(mutex) != 0) {
                    return -RT_ERROR;
                }

                /* exit critical and do schedule */
                rt_exit_critical();

                result = thread->error;

                /* lock mutex again */
                pthread_mutex_lock(mutex);
            }
        }
    }

    return result;
}
#endif


int pthread_condattr_init(pthread_condattr_t *attr)
{
    if (!attr) {
        return EINVAL;
    }

    attr->pshared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if (!attr) {
        return EINVAL;
    }

    return 0;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id)
{
    (void)attr;
    (void)clock_id;
    return 0;
}

int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id)
{
    (void)attr;
    (void)clock_id;
    return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
    if (!attr || !pshared) {
        return EINVAL;
    }

    *pshared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
    if ((pshared != PTHREAD_PROCESS_PRIVATE) &&
        (pshared != PTHREAD_PROCESS_SHARED)) {
        return EINVAL;
    }

    if (pshared != PTHREAD_PROCESS_PRIVATE) {
        return ENOSYS;
    }

    return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    if (cond == NULL) {
        ret = ENOMEM;
        return ret;
    }

    cond->attr = *attr;
    cond->wait_count = CONFIG_PTHREAD_COND_MAX_WAITERS;
    cond->wait_task = pvPortMalloc(sizeof(TaskHandle_t) * cond->wait_count);
    if (cond->wait_task != NULL) {
        for (int i = 0; i < cond->wait_count; i++ ) {
            cond->wait_task[i] = NULL;
        }
        cond->initialized = 0;
    } else {
        ret = ENOMEM;
    }

    return ret;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    char cond_name[RT_NAME_MAX];
    static rt_uint16_t cond_num = 0;

    if (cond == RT_NULL) {
        return EINVAL;
    }

    if ((attr != RT_NULL) && (*attr != PTHREAD_PROCESS_PRIVATE)) {
        return EINVAL;
    }

    rt_snprintf(cond_name, sizeof(cond_name), "cond%02d", cond_num++);

    if (attr == RT_NULL) {
        cond->attr.pshared = PTHREAD_PROCESS_PRIVATE;
    } else {
        cond->attr = *attr;
    }

    result = rt_sem_init(&cond->sem, cond_name, 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK) {
        return EINVAL;
    }

    /* detach the object from system object container */
    rt_object_detach(&(cond->sem.parent.parent));
    cond->sem.parent.parent.type = RT_Object_Class_Semaphore;

    return 0;
#endif
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    vPortFree(cond->wait_task);
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    if (cond == RT_NULL) {
        return EINVAL;
    }

    if (cond->attr.pshared == -1) {
        return 0;
    }

    if (!rt_list_isempty(&cond->sem.parent.suspend_thread)) {
        return EBUSY;
    }

__retry:
    result = rt_sem_trytake(&(cond->sem));
    if (result == EBUSY) {
        pthread_cond_broadcast(cond);
        goto __retry;
    }

    rt_memset(cond, 0, sizeof(pthread_cond_t));
    cond->attr.pshared = -1;
    return 0;
#endif
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int i = 0;
    if (!init_condstatic(cond)) {
        return ENOMEM;
    }

    taskENTER_CRITICAL();
    for (i = 0; i < cond->wait_count; i++) {
        if (cond->wait_task[i] != NULL) {
            xTaskNotify(cond->wait_task[i], 0, eNoAction);
            cond->wait_task[i] = NULL;
        }
    }

    taskEXIT_CRITICAL();
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;

    if (cond == RT_NULL) {
        return EINVAL;
    }

    if (cond->attr.pshared == -1) {
        pthread_cond_init(cond, RT_NULL);
    }

    while (1) {
        result = rt_sem_trytake(&(cond->sem));
        if (result == -RT_ETIMEOUT) {
            rt_sem_release(&(cond->sem));
        } else if (result == RT_EOK) {
            rt_sem_release(&(cond->sem));
            break;
        } else {
            return EINVAL;
        }
    }
    return 0;
#endif
}

int pthread_cond_signal(pthread_cond_t *cond)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    TaskHandle_t *task_to_notify = NULL;
    if (!init_condstatic(cond)) {
        return ENOMEM;
    }

    taskENTER_CRITICAL();

    for (int i = 0; i < cond->wait_count; i++) {
        if (cond->wait_task[i] != NULL) {
            if (task_to_notify == NULL) {
                task_to_notify = &cond->wait_task[i];
                continue;
            }

            if (uxTaskPriorityGet(*task_to_notify) < uxTaskPriorityGet(cond->wait_task[i])){
                task_to_notify = &cond->wait_task[i];
            }
        }
    }

    if (task_to_notify != NULL) {
        xTaskNotify(*task_to_notify, 0, eNoAction);
        *task_to_notify = NULL;
    }

    taskEXIT_CRITICAL();
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_base_t temp;
    rt_err_t result;

    if (cond == RT_NULL) {
        return EINVAL;
    }

    if (cond->attr == -1) {
        pthread_cond_init(cond, RT_NULL);
    }

    temp = rt_hw_interrupt_disable();
    if (rt_list_isempty(&cond->sem.parent.suspend_thread)) {
        /* enable interrupt */
        rt_hw_interrupt_enable(temp);
        return 0;
    } else {
        /* enable interrupt */
        rt_hw_interrupt_enable(temp);
        result = rt_sem_release(&(cond->sem));
        if (result == RT_EOK) {
            return 0;
        }
        return 0;
    }
#endif
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    unsigned local_waitThread;
    int ret = 0;
    TickType_t delay = portMAX_DELAY;
    int set = -1;
    int i = 0;

    if (!init_condstatic(cond)) {
        return ENOMEM;
    }

    if (abstime != NULL) {
        struct timespec cur = { .tv_nsec = 0, .tv_sec = 0, };
        if (clock_gettime(CLOCK_REALTIME, &cur) != 0) {
            ret = EINVAL;
            return ret;
        } else {
            ret = utils_timespec_todeltaticks(abstime, &cur, &delay);
            if (ret != 0) {
                return EINVAL;
            }
        }
    }

    taskENTER_CRITICAL();
    for (i = 0; i < cond->wait_count; i++) {
        if (cond->wait_task[i] == NULL) {
            cond->wait_task[i] = xTaskGetCurrentTaskHandle();
            set = 0;
            break;
        }
    }
    taskEXIT_CRITICAL();

    if (set != 0) {
        return ENOMEM;
    }

    ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
        return ret;
    }

    if (xTaskNotifyWait(0, 0, NULL, delay) == pdPASS) {
        /* When successful, relock mutex. */
        ret = pthread_mutex_lock(mutex);
    } else {
        /* Timeout. Relock mutex and decrement number of waiting threads. */
        ret = ETIMEDOUT;
        pthread_mutex_lock(mutex);
    }

    taskENTER_CRITICAL();
    for (i = 0; i < cond->wait_count; i++) {
        if (cond->wait_task[i] == xTaskGetCurrentTaskHandle()) {
            cond->wait_task[i] = NULL;
            break;
        }
    }
    taskEXIT_CRITICAL();

    return ret;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;

__retry:
    result = _pthread_cond_timedwait(cond, mutex, RT_WAITING_FOREVER);
    if (result == RT_EOK) {
        return 0;
    } else if (result == -RT_EINTR) {
        /* https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_wait.html
         * These functions shall not return an error code of [EINTR].
         */
        goto __retry;
    }

    return EINVAL;
#endif
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return pthread_cond_timedwait(cond, mutex, (const struct timespec *)NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)

    int timeout;
    rt_err_t result;

    timeout = rt_timespec_to_tick(abstime);
    result = _pthread_cond_timedwait(cond, mutex, timeout);
    if (result == RT_EOK){
        return 0;
    }

    if (result == -RT_ETIMEOUT) {
        return ETIMEDOUT;
    }

    return EINVAL;
#endif
}
