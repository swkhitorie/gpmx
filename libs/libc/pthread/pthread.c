#include <stddef.h>
#include <string.h>
#include "pthread.h"
#include "errno.h"

int g_irqerrno;

typedef struct __pthread_obj
{
    pthread_attr_t attr;
    void *(*start)(void *);
    void *arg;
    void *ret;
    int pterrno;
    TaskHandle_t handle;             /**< FreeRTOS task handle. */
    StaticSemaphore_t join_barrier;  /**< Synchronizes the two callers of pthread_join. */
    StaticSemaphore_t join_mutex;    /**< Ensures that only one other thread may join this thread. */
} pthread_obj_t;

static void exit_thread( void )
{
    pthread_obj_t *p = (pthread_obj_t *)pthread_self();

    if (p->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
        (void)xSemaphoreGive((SemaphoreHandle_t)&p->join_barrier);
        vTaskSuspend(NULL);
    } else {
        vPortFree(p);
        vTaskDelete(NULL);
    }
}

static void run_thread(void *xarg)
{
    pthread_obj_t *p = (pthread_obj_t *)xarg;
    p->ret = p->start((void *)p->arg);
    exit_thread();
}

int *get_errno_ptr()
{
    pthread_obj_t *p = (pthread_obj_t *)pthread_self();
    return p->pterrno;
}

int pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*start)(void *), void *arg)
{
    int ret = 0;
    pthread_obj_t *pthread = NULL;
    struct sched_param sched_p = { .sched_priority = tskIDLE_PRIORITY };
    pthread_attr_t attr_default;

    pthread_attr_init(&attr_default);

    pthread = (pthread_obj_t *)pvPortMalloc(sizeof(pthread_obj_t));
    if (pthread == NULL) {
        ret = ENOMEM;
        goto pthread_create_exit;
    }

    pthread->attr = (attr == NULL) ? attr_default : *attr;

    sched_p.sched_priority = (int)pthread->attr.schedparam.sched_priority;
    pthread->arg = arg;
    pthread->start = start;
    if (pthread->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
        xSemaphoreCreateMutexStatic(&pthread->join_mutex);
        xSemaphoreCreateBinaryStatic(&pthread->join_barrier);
    }

    vTaskSuspendAll();

    if (xTaskCreate(run_thread,"pthread",
        (uint16_t)(pthread->attr.stacksize/sizeof(StackType_t)), (void *)pthread,
        sched_p.sched_priority, &pthread->handle) != pdPASS) {
        vPortFree(pthread);
        ret = EAGAIN;
    } else {
        vTaskSetApplicationTaskTag(pthread->handle, (TaskHookFunction_t)pthread);
        *tid = (pthread_t)pthread;
    }
    xTaskResumeAll();
pthread_create_exit:
    return ret;
}

pthread_t pthread_self(void)
{
    return (pthread_t)xTaskGetApplicationTaskTag(NULL);
}

void pthread_exit(void *value_ptr)
{
    pthread_obj_t *p = (pthread_obj_t *)pthread_self();
    p->ret = value_ptr;
    exit_thread();
}

int pthread_join(pthread_t pthread, void **retval)
{
    int ret = 0;
    pthread_obj_t *p = (pthread_obj_t *)pthread;

    if (p->attr.detachstate != PTHREAD_CREATE_JOINABLE) {
        ret = EDEADLK;
        goto pthread_join_exit;
    }

    if (xSemaphoreTake((SemaphoreHandle_t)&p->join_mutex, 0) != pdPASS) {
        ret = EDEADLK;
        goto pthread_join_exit;
    }

    if (pthread_equal(pthread_self(), pthread) != 0) {
        ret = EDEADLK;
        goto pthread_join_exit;
    }

    (void)xSemaphoreTake((SemaphoreHandle_t)&p->join_barrier, portMAX_DELAY);
    vTaskSuspendAll();
    (void)xSemaphoreGive((SemaphoreHandle_t)&p->join_barrier);
    vSemaphoreDelete((SemaphoreHandle_t)&p->join_barrier);
    (void)xSemaphoreGive((SemaphoreHandle_t)&p->join_mutex);
    vSemaphoreDelete((SemaphoreHandle_t)&p->join_mutex);
    vTaskDelete(p->handle);

    if (retval != NULL) {
        *retval = p->ret;
    }
    vPortFree(p);
    xTaskResumeAll();
pthread_join_exit:
    return ret;
}

int pthread_cancel(pthread_t thread)
{ 
    return -1; 
}

int pthread_kill(pthread_t thread, int sig)
{ 
    return -1; 
}

int pthread_setname_np(pthread_t thread, const char *name) 
{
    pthread_obj_t *p = (pthread_obj_t *)pthread_self();
    pcTaskSetName(p->handle, name);
    return 0;
}

int pthread_getname_np(pthread_t thread, char *name, int namelen)
{
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
}

int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param)
{
    int ret = 0;
    pthread_obj_t *p = (pthread_obj_t *)thread;

    *policy = SCHED_OTHER;
    param->sched_priority = (int)p->attr.schedparam.sched_priority;
    return ret;
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    int ret = 0;
    pthread_obj_t *p = (pthread_obj_t *)thread;
    (void)policy;

    ret = pthread_attr_setschedparam((pthread_attr_t *)&p->attr, param);
    if (ret == 0) {
        vTaskPrioritySet(p->handle, param->sched_priority);
    }
    return ret;
}
