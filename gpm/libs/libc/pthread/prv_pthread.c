#include <gpmx/config.h>
#include "prv_pthread.h"
#include <string.h>

#if defined(CONFIG_RTTNANO_ENABLE)
RT_DEFINE_SPINLOCK(pth_lock);
_pthread_data_t *pth_table[PTHREAD_NUM_MAX] = {NULL};

_pthread_data_t *_pthread_get_data(pthread_t thread)
{
    RT_DECLARE_SPINLOCK(pth_lock);
    _pthread_data_t *ptd;

    if (thread >= PTHREAD_NUM_MAX) {
        return NULL;
    }

    rt_hw_spin_lock(&pth_lock);
    ptd = pth_table[thread];
    rt_hw_spin_unlock(&pth_lock);

    if (ptd && ptd->magic == PTHREAD_MAGIC) {
        return ptd;
    }

    return NULL;
}

pthread_t _pthread_data_get_pth(_pthread_data_t *ptd)
{
    int index;
    RT_DECLARE_SPINLOCK(pth_lock);

    rt_hw_spin_lock(&pth_lock);
    for (index = 0; index < PTHREAD_NUM_MAX; index ++) {
        if (pth_table[index] == ptd) break;
    }
    rt_hw_spin_unlock(&pth_lock);

    return index;
}

pthread_t _pthread_data_create(void)
{
    int index;
    _pthread_data_t *ptd = NULL;
    RT_DECLARE_SPINLOCK(pth_lock);

    ptd = (_pthread_data_t*)rt_malloc(sizeof(_pthread_data_t));
    if (!ptd) {
        return PTHREAD_NUM_MAX;
    } 

    memset(ptd, 0x0, sizeof(_pthread_data_t));
    ptd->canceled = 0;
    ptd->cancelstate = PTHREAD_CANCEL_DISABLE;
    ptd->canceltype = PTHREAD_CANCEL_DEFERRED;
    ptd->magic = PTHREAD_MAGIC;

    rt_hw_spin_lock(&pth_lock);
    for (index = 0; index < PTHREAD_NUM_MAX; index ++) {
        if (pth_table[index] == NULL) {
            pth_table[index] = ptd;
            break;
        }
    }
    rt_hw_spin_unlock(&pth_lock);

    if (index == PTHREAD_NUM_MAX) {
        ptd->magic = 0x0;
        rt_free(ptd);
    }

    return index;
}

void _pthread_data_destroy(_pthread_data_t *ptd)
{
    RT_DECLARE_SPINLOCK(pth_lock);

    extern _pthread_key_data_t _thread_keys[PTHREAD_KEY_MAX];
    pthread_t pth;

    if (ptd) {

        /* if this thread create the local thread data,
         * destruct thread local key
         */
        if (ptd->tls != RT_NULL) {
            void *data;
            rt_uint32_t index;
            for (index = 0; index < PTHREAD_KEY_MAX; index ++) {
                if (_thread_keys[index].is_used) {
                    data = ptd->tls[index];
                    if (data && _thread_keys[index].destructor) {
                        _thread_keys[index].destructor(data);
                    }
                }
            }

            rt_free(ptd->tls);
            ptd->tls = RT_NULL;
        }

        pth  = _pthread_data_get_pth(ptd);
        rt_hw_spin_lock(&pth_lock);
        pth_table[pth] = NULL;
        rt_hw_spin_unlock(&pth_lock);

        if (ptd->joinable_sem != RT_NULL) {
            rt_sem_delete(ptd->joinable_sem);
            ptd->joinable_sem = RT_NULL;
        }

        ptd->magic = 0x0;
        ptd->tid->pthread_data = RT_NULL;
        rt_free(ptd);
    }
}

void _pthread_cleanup(rt_thread_t tid)
{
    /* clear cleanup function */
    tid->cleanup = RT_NULL;

    /* restore tid stack */
    rt_free(tid->stack_addr);

    /* clear the "ptd->tid->pthread_data" */
    tid->pthread_data = RT_NULL;

    /* restore tid control block */
    rt_free(tid);
}

void pthread_entry_stub(void *parameter)
{
    void *value;
    _pthread_data_t *ptd;

    ptd = (_pthread_data_t *)parameter;

    value = ptd->thread_entry(ptd->thread_parameter);

    if (ptd->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
        ptd->return_value = value;
        rt_sem_release(ptd->joinable_sem);
    } else {
        _pthread_data_destroy(ptd);
    }
}

#endif

#if defined(CONFIG_FREERTOS_ENABLE)

void prv_exit_thread()
{
    pthread_obj_t *p = (pthread_obj_t *)pthread_self();

    vTaskSuspendAll();

    if (p->attr.detachstate == PTHREAD_CREATE_JOINABLE) {
        xSemaphoreGive((SemaphoreHandle_t)&p->join_barrier);
        xTaskResumeAll();
        vTaskSuspend(NULL);
    } else {
        xTaskResumeAll();
        vPortFree(p);
        vTaskDelete(NULL);
    }
}

void prv_run_thread(void *xarg)
{
    pthread_obj_t *p = (pthread_obj_t *)xarg;
    p->ret = p->start((void *)p->arg);
    prv_exit_thread();
}
#endif


