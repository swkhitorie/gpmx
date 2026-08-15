#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <gpmx/config.h>

#include "utils.h"
#include "prv_pthread.h"

static int concurrency_level;
static int pthread_counter = 0;

static int g_irqerrno = 0;
int *__errno()
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (xPortIsInsideInterrupt() != pdTRUE) {
        pthread_obj_t *p = (pthread_obj_t *)pthread_self();
        if (p) {
            return &p->pterrno;
        }
    }

    return &g_irqerrno;
#elif defined(CONFIG_RTTNANO_ENABLE)

    return _rt_errno();
#endif
}

int pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*start)(void *), void *arg)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    char name[16];
    static uint16_t pthread_number = 0;
    pthread_obj_t *pthread = NULL;
    struct sched_param sched_p = { .sched_priority = tskIDLE_PRIORITY };
    pthread_attr_t attr_default;

    pthread_attr_init(&attr_default);

    pthread = (pthread_obj_t *)pvPortMalloc(sizeof(pthread_obj_t));
    if (pthread == NULL) {
        ret = ENOMEM;
        goto pthread_create_exit;
    }

    snprintf(name, sizeof(name), "pth%02d", pthread_number++);

    pthread->attr = (attr == NULL) ? attr_default : *attr;

    sched_p.sched_priority = (int)pthread->attr.schedparam.sched_priority;
    pthread->arg = arg;
    pthread->start = start;
    if (pthread->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
        xSemaphoreCreateMutexStatic(&pthread->join_mutex);
        xSemaphoreCreateBinaryStatic(&pthread->join_barrier);
    }

    vTaskSuspendAll();

    files_initlist(&pthread->tg_filelist);
    pthread->tg_filelist.fl_files = NULL;
    pthread->tg_filelist.fl_rows = 0;
    if (pthread_counter > 0) {
        pthread_obj_t *p_self = (pthread_obj_t *)pthread_self();
        if (p_self) {
            files_duplist(&p_self->tg_filelist, &pthread->tg_filelist);
        }
    }

    if (xTaskCreate(prv_run_thread, name,
        (uint16_t)(pthread->attr.stacksize/sizeof(StackType_t)), (void *)pthread,
        sched_p.sched_priority, &pthread->handle) != pdPASS) {
        vPortFree(pthread);
        ret = EAGAIN;
    } else {
        vTaskSetApplicationTaskTag(pthread->handle, (TaskHookFunction_t)pthread);
        *tid = (pthread_t)pthread;
    }

    pthread_counter++;

    xTaskResumeAll();
pthread_create_exit:
    return ret;
#elif defined(CONFIG_RTTNANO_ENABLE)

    int ret = 0;
    void *stack;
    char name[RT_NAME_MAX];
    static rt_uint16_t pthread_number = 0;

    pthread_t pth_id;
    _pthread_data_t *ptd;

    /* pid shall be provided */
    RT_ASSERT(pid != RT_NULL);

    pth_id = _pthread_data_create();
    if (pth_id == PTHREAD_NUM_MAX) {
        ret = ENOMEM;
        goto __exit;
    }

    ptd = _pthread_get_data(pth_id);
    RT_ASSERT(ptd != RT_NULL);

    if (attr != RT_NULL) {
        ptd->attr = *attr;
    } else {
        /* use default attribute */
        pthread_attr_init(&ptd->attr);
    }

    if (ptd->attr.stacksize == 0) {
        ret = EINVAL;
        goto __exit;
    }

    rt_snprintf(name, sizeof(name), "pth%02d", pthread_number ++);

    /* pthread is a static thread object */
    ptd->tid = (rt_thread_t) rt_malloc(sizeof(struct rt_thread));
    if (ptd->tid == RT_NULL) {
        ret = ENOMEM;
        goto __exit;
    }
    memset(ptd->tid, 0, sizeof(struct rt_thread));

    if (ptd->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
        ptd->joinable_sem = rt_sem_create(name, 0, RT_IPC_FLAG_FIFO);
        if (ptd->joinable_sem == RT_NULL) {
            ret = ENOMEM;
            goto __exit;
        }
    } else {
        ptd->joinable_sem = RT_NULL;
    }

    /* set parameter */
    ptd->thread_entry = start;
    ptd->thread_parameter = arg;

    /* stack */
    if (ptd->attr.stackaddr == 0) {
        stack = (void *)rt_malloc(ptd->attr.stacksize);
    } else {
        stack = (void *)(ptd->attr.stackaddr);
    }

    if (stack == RT_NULL) {
        ret = ENOMEM;
        goto __exit;
    }

    files_initlist(&ptd->tg_filelist);
    ptd->tg_filelist.fl_files = NULL;
    ptd->tg_filelist.fl_rows = 0;
    if (pthread_counter > 0) {
        rt_thread_t tid_self;
        _pthread_data_t *ptd_self;

        tid_self = rt_thread_self();
        if (tid_self == NULL) {
            return -1;
        }

        ptd_self = (_pthread_data_t *)rt_thread_self()->pthread_data;
        files_duplist(&ptd_self->tg_filelist, &ptd->tg_filelist);
    }

    ptd->attr.schedparam.sched_priority = 
        (RT_THREAD_PRIORITY_MAX - 1) - ptd->attr.schedparam.sched_priority;

    /* initial this pthread to system */
    if (rt_thread_init(ptd->tid, name, pthread_entry_stub, ptd,
                        stack, ptd->attr.stacksize,
                        ptd->attr.schedparam.sched_priority, 20) != RT_EOK) {
        ret = EINVAL;
        goto __exit;
    }

    pthread_counter++;

    /* set pthread id */
    *tid = pth_id;

    /* set pthread cleanup function and ptd data */
    ptd->tid->cleanup = _pthread_cleanup;
    ptd->tid->pthread_data = (void *)ptd;

    /* start thread */
    if (rt_thread_startup(ptd->tid) == RT_EOK) {
        return 0;
    }

    /* start thread failed */
    rt_thread_detach(ptd->tid);
    ret = EINVAL;

__exit:
    if (pth_id != PTHREAD_NUM_MAX) {
        _pthread_data_destroy(ptd);
    }
    return ret;
#endif
}

int pthread_detach(pthread_t thread)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    pthread_obj_t *p = (pthread_obj_t *)thread;
    eTaskState pthread_state;

    if (p->attr.detachstate != PTHREAD_CREATE_JOINABLE) {
        ret = EDEADLK;
        goto pthread_detach_exit;
    }

    vTaskSuspendAll();
    pthread_state = eTaskGetState(p->handle);
    /* Thread has been deleted or is invalid. */
    if ((pthread_state == eDeleted) || (pthread_state == eInvalid)) {
        ret = EINVAL;
        goto pthread_detach_exit;
    }

    xSemaphoreGive((SemaphoreHandle_t)&p->join_barrier);
    vSemaphoreDelete((SemaphoreHandle_t)&p->join_barrier);
    xSemaphoreGive((SemaphoreHandle_t)&p->join_mutex);
    vSemaphoreDelete((SemaphoreHandle_t)&p->join_mutex);

    if (pthread_state == eSuspended) {
        vTaskDelete(p->handle);
        vPortFree(p);
    } else {
        pthread_attr_setdetachstate((pthread_attr_t *)&p->attr, PTHREAD_CREATE_DETACHED);
    }

pthread_detach_exit:
    xTaskResumeAll();
    return ret;
#elif defined(CONFIG_RTTNANO_ENABLE)

    int ret = 0;
    _pthread_data_t *ptd = _pthread_get_data(thread);
    if (ptd == RT_NULL) {
        ret = EINVAL;
        goto __exit;
    }

    if (ptd->attr.detachstate == PTHREAD_CREATE_DETACHED) {
        ret = EINVAL;
        goto __exit;
    }

    if ((ptd->tid->stat & RT_THREAD_STAT_MASK) == RT_THREAD_CLOSE) {
        /* destroy this pthread */
        _pthread_data_destroy(ptd);
        goto __exit;
    } else {
        ptd->attr.detachstate = PTHREAD_CREATE_DETACHED;

        /* detach joinable semaphore */
        if (ptd->joinable_sem) {
            rt_sem_delete(ptd->joinable_sem);
            ptd->joinable_sem = RT_NULL;
        }
    }

__exit:
    return ret;
#endif
}

int pthread_join(pthread_t thread, void **retval)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    pthread_obj_t *p = (pthread_obj_t *)thread;

    if (p->attr.detachstate != PTHREAD_CREATE_JOINABLE) {
        ret = EDEADLK;
        goto pthread_join_exit;
    }

    if (xSemaphoreTake((SemaphoreHandle_t)&p->join_mutex, 0) != pdPASS) {
        ret = EDEADLK;
        goto pthread_join_exit;
    }

    if (pthread_equal(pthread_self(), thread) != 0) {
        ret = EDEADLK;
        goto pthread_join_exit;
    }

    xSemaphoreTake((SemaphoreHandle_t)&p->join_barrier, portMAX_DELAY);
    vTaskSuspendAll();

    xSemaphoreGive((SemaphoreHandle_t)&p->join_barrier);
    vSemaphoreDelete((SemaphoreHandle_t)&p->join_barrier);
    xSemaphoreGive((SemaphoreHandle_t)&p->join_mutex);
    vSemaphoreDelete((SemaphoreHandle_t)&p->join_mutex);
    vTaskDelete(p->handle);

    if (retval != NULL) {
        *retval = p->ret;
    }
    vPortFree(p);
    xTaskResumeAll();
pthread_join_exit:
    return ret;
#elif defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    rt_err_t result;

    ptd = _pthread_get_data(thread);
    if (ptd == RT_NULL) {
        return EINVAL;
    }

    if (ptd->tid == rt_thread_self()) {
        /* join self */
        return EDEADLK;
    }

    if (ptd->attr.detachstate == PTHREAD_CREATE_DETACHED) {
        return EINVAL;
    }

    result = rt_sem_take(ptd->joinable_sem, RT_WAITING_FOREVER);
    if (result == RT_EOK) {
        /* get return value */
        if (retval != RT_NULL) {
            *retval = ptd->return_value;
        }

        /* destroy this pthread */
        _pthread_data_destroy(ptd);
    } else {
        return ESRCH;
    }

    return 0;
#endif
}

pthread_t pthread_self(void)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return (pthread_t)xTaskGetApplicationTaskTag(NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_thread_t tid;
    _pthread_data_t *ptd;

    tid = rt_thread_self();
    if (tid == NULL) {
        return PTHREAD_NUM_MAX;
    }

    ptd = (_pthread_data_t *)rt_thread_self()->pthread_data;
    RT_ASSERT(ptd != RT_NULL);

    return _pthread_data_get_pth(ptd);
#endif
}

int pthread_getcpuclockid(pthread_t thread, clockid_t *clock_id)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return xTaskGetTickCount();
#elif defined(CONFIG_RTTNANO_ENABLE)

    if(_pthread_get_data(thread) == NULL) {
        return EINVAL;
    }
    *clock_id = (clockid_t)rt_tick_get();
    return 0;
#endif
}

int pthread_getconcurrency(void)
{
    return concurrency_level;
}

int pthread_setconcurrency(int new_level)
{
    concurrency_level = new_level;
    return 0;
}

int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    pthread_obj_t *p = (pthread_obj_t *)thread;

    pthread_attr_getschedpolicy(&p->attr, policy);
    pthread_attr_setschedparam(&p->attr, param);
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    ptd = _pthread_get_data(thread);

    pthread_attr_getschedpolicy(&ptd->attr, policy);
    pthread_attr_getschedparam(&ptd->attr, param);
    return 0;
#endif
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    pthread_obj_t *p = (pthread_obj_t *)thread;

    pthread_attr_setschedpolicy(&p->attr, policy);
    pthread_attr_setschedparam(&p->attr, param);
    vTaskPrioritySet(p->handle, param->sched_priority);
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    ptd = _pthread_get_data(thread);

    pthread_attr_setschedpolicy(&ptd->attr, policy);
    pthread_attr_setschedparam(&ptd->attr, param);
    return 0;
#endif
}

int pthread_setschedprio(pthread_t thread, int prio)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    pthread_obj_t *p = (pthread_obj_t *)thread;
    struct sched_param param;
    param.sched_priority = prio;

    pthread_attr_setschedparam(&p->attr, &param);
    vTaskPrioritySet(p->handle, param.sched_priority);
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    struct sched_param param;

    ptd = _pthread_get_data(thread);
    param.sched_priority = prio;
    pthread_attr_setschedparam(&ptd->attr, &param);

    return 0;
#endif
}

void pthread_exit(void *value)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    pthread_obj_t *p = (pthread_obj_t *)pthread_self();
    p->ret = value;
    prv_exit_thread();
#elif defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;
    rt_thread_t tid;

    if (rt_thread_self() == RT_NULL) {
        return;
    }

    ptd = (_pthread_data_t *)rt_thread_self()->pthread_data;

    rt_enter_critical();
    ptd->cancelstate = PTHREAD_CANCEL_DISABLE;
    ptd->return_value = value;
    rt_exit_critical();

    /*
    * When use pthread_exit to exit.
    * invoke pushed cleanup
    */
    while (ptd->cleanup != RT_NULL) {
        cleanup = ptd->cleanup;
        ptd->cleanup = cleanup->next;

        cleanup->cleanup_func(cleanup->parameter);
        /* release this cleanup function */
        rt_free(cleanup);
    }

    /* get the info aboult "tid" early */
    tid = ptd->tid;

    /* According to "detachstate" to whether or not to recycle resource immediately */
    if (ptd->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
        /* set value */
        rt_sem_release(ptd->joinable_sem);
    } else {
        /* release pthread resource */
        _pthread_data_destroy(ptd);
    }

    /*
        * second: detach thread.
        * this thread will be removed from scheduler list
        * and because there is a cleanup function in the
        * thread (pthread_cleanup), it will move to defunct
        * thread list and wait for handling in idle thread.
        */
    rt_thread_detach(tid);

    /* reschedule thread */
    rt_schedule();
#endif
}

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
#if defined(CONFIG_FREERTOS_ENABLE)

    portENTER_CRITICAL();
    if (!(*once_control)) {
        /* call routine once */
        *once_control = 1;
        portEXIT_CRITICAL();

        init_routine();
    }
    portEXIT_CRITICAL();

    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    RT_ASSERT(once_control != RT_NULL);
    RT_ASSERT(init_routine != RT_NULL);

    rt_enter_critical();
    if (!(*once_control)) {
        /* call routine once */
        *once_control = 1;
        rt_exit_critical();

        init_routine();
    }
    rt_exit_critical();

    return 0;
#endif
}

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
    return EOPNOTSUPP;
}

int pthread_kill(pthread_t thread, int sig)
{
#if defined(CONFIG_RTTNANO_ENABLE) && defined(RT_USING_SIGNALS)

    _pthread_data_t *ptd;
    int ret;

    ptd = _pthread_get_data(thread);
    if (ptd) {
        ret = rt_thread_kill(ptd->tid, sig);
        if (ret == -RT_EINVAL) {
            return EINVAL;
        }

        return ret;
    }

    return ESRCH;
#else
    return ENOSYS;
#endif
}

void pthread_cleanup_pop(int execute)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;

    if (rt_thread_self() == NULL) return;

    ptd = (_pthread_data_t *)rt_thread_self()->pthread_data;
    RT_ASSERT(ptd != RT_NULL);

    if (execute) {
        rt_enter_critical();
        cleanup = ptd->cleanup;
        if (cleanup) {
            ptd->cleanup = cleanup->next;
        }
        rt_exit_critical();

        if (cleanup) {
            cleanup->cleanup_func(cleanup->parameter);
            rt_free(cleanup);
        }
    }
#endif
}

void pthread_cleanup_push(void (*routine)(void *), void *arg)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;

    if (rt_thread_self() == NULL) return;

    ptd = (_pthread_data_t *)rt_thread_self()->pthread_data;
    RT_ASSERT(ptd != RT_NULL);

    cleanup = (_pthread_cleanup_t *)rt_malloc(sizeof(_pthread_cleanup_t));
    if (cleanup != RT_NULL) {
        cleanup->cleanup_func = routine;
        cleanup->parameter = arg;

        rt_enter_critical();
        cleanup->next = ptd->cleanup;
        ptd->cleanup = cleanup;
        rt_exit_critical();
    }
#endif
}

int pthread_setcancelstate(int state, int *oldstate)
{
#if defined(CONFIG_RTTNANO_ENABLE)
    _pthread_data_t *ptd;

    if (rt_thread_self() == NULL) return EINVAL;

    ptd = (_pthread_data_t *)rt_thread_self()->pthread_data;
    RT_ASSERT(ptd != RT_NULL);

    if ((state == PTHREAD_CANCEL_ENABLE) || (state == PTHREAD_CANCEL_DISABLE)) {
        if (oldstate) {
            *oldstate = ptd->cancelstate;
        }
        ptd->cancelstate = state;

        return 0;
    }

    return EINVAL;
#else
    return EINVAL;
#endif
}

int pthread_setcanceltype(int type, int *oldtype)
{
#if defined(CONFIG_RTTNANO_ENABLE)
    _pthread_data_t *ptd;

    if (rt_thread_self() == NULL) return EINVAL;

    ptd = (_pthread_data_t *)rt_thread_self()->pthread_data;
    RT_ASSERT(ptd != RT_NULL);

    if ((type != PTHREAD_CANCEL_DEFERRED) && (type != PTHREAD_CANCEL_ASYNCHRONOUS)) {
        return EINVAL;
    }

    if (oldtype) {
        *oldtype = ptd->canceltype;
    }
    ptd->canceltype = type;

    return 0;
#else
    return EINVAL;
#endif
}

void pthread_testcancel(void)
{
#if defined(CONFIG_RTTNANO_ENABLE)
    int cancel = 0;
    _pthread_data_t *ptd;

    if (rt_thread_self() == NULL) return;

    ptd = (_pthread_data_t *)rt_thread_self()->pthread_data;
    RT_ASSERT(ptd != RT_NULL);

    if (ptd->cancelstate == PTHREAD_CANCEL_ENABLE) {
        cancel = ptd->canceled;
    }
    if (cancel) {
        pthread_exit((void *)PTHREAD_CANCELED);
    }
#endif
}

int pthread_cancel(pthread_t thread)
{
#if defined(CONFIG_RTTNANO_ENABLE)
    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;
    rt_thread_t tid;

    ptd = _pthread_get_data(thread);
    if (ptd == RT_NULL) {
        return EINVAL;
    }

    tid = ptd->tid;

    /* cancel self */
    if (ptd->tid == rt_thread_self()) {
        return 0;
    }

    /* set canceled */
    if (ptd->cancelstate == PTHREAD_CANCEL_ENABLE)  {
        ptd->canceled = 1;
        if (ptd->canceltype == PTHREAD_CANCEL_ASYNCHRONOUS) {
            /*
             * When use pthread_cancel to exit.
             * invoke pushed cleanup
             */
            while (ptd->cleanup != RT_NULL) {
                cleanup = ptd->cleanup;
                ptd->cleanup = cleanup->next;

                cleanup->cleanup_func(cleanup->parameter);
                rt_free(cleanup);
            }

            /* According to "detachstate" to whether or not to recycle resource immediately */
            if (ptd->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
                /* set value */
                rt_sem_release(ptd->joinable_sem);
            } else {
                /* release pthread resource */
                _pthread_data_destroy(ptd);
            }

            /*
             * second: detach thread.
             * this thread will be removed from scheduler list
             * and because there is a cleanup function in the
             * thread (pthread_cleanup), it will move to defunct
             * thread list and wait for handling in idle thread.
             */
            rt_thread_detach(tid);
        }
    }

    return 0;
#else
    return EINVAL;
#endif
}

int pthread_setname_np(pthread_t thread, const char *name) 
{
#if defined(CONFIG_FREERTOS_ENABLE)

    pthread_obj_t *p = (pthread_obj_t *)pthread_self();
    pcTaskSetName(p->handle, name);
    return 0;
#elif defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    rt_thread_t tid;

    ptd = _pthread_get_data(thread);
    if (ptd == RT_NULL) {
        return EINVAL;
    }

    tid = ptd->tid;
    rt_thread_rename(tid, name);

    return EINVAL;
#endif
}

int pthread_getname_np(pthread_t thread, char *name, int namelen)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    char *tmp = NULL;
    int i = 0;
    pthread_obj_t *p = (pthread_obj_t *)pthread_self();
    
    if (namelen > 16) return -1;

    tmp = pcTaskGetName(p->handle);
    if (!tmp) return -1;
    for (i = 0; i < namelen; i++) {
        if (tmp[i] == 0x00 || tmp[i] == '\n') {
            break;
        }
        name[i] = tmp[i];
    }
    return 0; 
#elif defined(CONFIG_RTTNANO_ENABLE)

    _pthread_data_t *ptd;
    rt_thread_t tid;

    ptd = _pthread_get_data(thread);
    if (ptd == RT_NULL) {
        return EINVAL;
    }

    tid = ptd->tid;
    for (int i = 0; i < RT_NAME_MAX; i++) {
        name[i] = tid->name[i];
    }

    return 0;
#endif
}

struct filelist *pt_sched_get_files()
{
#if defined(CONFIG_FREERTOS_ENABLE)
    pthread_obj_t *p = (pthread_obj_t *)pthread_self();
    return &p->tg_filelist;
#elif defined(CONFIG_RTTNANO_ENABLE)
    pthread_t thread = pthread_self();
    _pthread_data_t *ptd = _pthread_get_data(thread);
    return &ptd->tg_filelist;
#endif
}

struct streamlist *pt_sched_get_streams(void)
{
    return NULL;
}
