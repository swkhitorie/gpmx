#include <semaphore.h>
#include <string.h>
#include "prv_sem.h"
#include <gpmx/config.h>

#if defined(CONFIG_RTTNANO_ENABLE)

static sem_t *posix_sem_list = RT_NULL;
static struct rt_semaphore _vposix_sem_lock;

void posix_sem_lock()
{
    rt_sem_take(&_vposix_sem_lock, RT_WAITING_FOREVER);
}

void posix_sem_unlock()
{
    rt_sem_release(&_vposix_sem_lock);
}

int posix_sem_system_init(void)
{
    rt_sem_init(&_vposix_sem_lock, "psem", 1, RT_IPC_FLAG_FIFO);
    return 0;
}
INIT_COMPONENT_EXPORT(posix_sem_system_init);

void posix_sem_insert(sem_t *psem)
{
    psem->next = posix_sem_list;
    posix_sem_list = psem;
}

/**
 * Deletes a semaphore from the linked list of semaphores.
 */
void posix_sem_delete(sem_t *psem)
{
    sem_t *iter;

    if (posix_sem_list == psem) {
        posix_sem_list = psem->next;

#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
        if (psem->protocol == SEM_PRIO_INHERIT) {
            rt_mutex_delete(psem->mutex);
        } else {
            rt_sem_delete(psem->sem);
        }
#else
        rt_sem_delete(psem->sem);
#endif
        if(psem->unamed == 0) {
            rt_free(psem);
        }

        return;
    }

    for (iter = posix_sem_list; iter->next != RT_NULL; iter = iter->next) {

        if (iter->next == psem) {
            /* delete this mq */
            if (psem->next != RT_NULL)
                iter->next = psem->next;
            else
                iter->next = RT_NULL;

            /* delete RT-Thread mqueue */
#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
            if (psem->protocol == SEM_PRIO_INHERIT) {
                rt_mutex_delete(psem->mutex);
            } else {
                rt_sem_delete(psem->sem);
            }
#else
            rt_sem_delete(psem->sem);
#endif

            if(psem->unamed == 0) {
                rt_free(psem);
            }

            return ;
        }
    }
}

/**
 * Finds a semaphore by name in the linked list of semaphores.
 */
sem_t *posix_sem_find(const char* name)
{
    sem_t *iter;
    rt_object_t object;

    for (iter = posix_sem_list; iter != RT_NULL; iter = iter->next) {
        object = (rt_object_t)iter->sem;

        if (strncmp(object->name, name, RT_NAME_MAX) == 0) {
            return iter;
        }
    }

    return RT_NULL;
}
#else
int posix_sem_system_init()
{
    return -1;
}

void posix_sem_insert(sem_t *psem)
{
    (void)psem;
}

void posix_sem_delete(sem_t *psem)
{
    (void)psem;
}

sem_t *posix_sem_find(const char* name)
{
    return ((void*)0);
}

void posix_sem_lock()
{

}

void posix_sem_unlock()
{

}

#endif
