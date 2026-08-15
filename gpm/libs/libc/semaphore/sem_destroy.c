#include <semaphore.h>
#include "prv_sem.h"
#include <gpmx/config.h>

int sem_destroy(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    if ((!sem) || !(sem->unamed)) {
        rt_set_errno(EINVAL);
        return -1;
    }

    posix_sem_lock();

    /* Check if it is in a switchable state */
#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
    if (sem->protocol == SEM_PRIO_INHERIT) {
        if (sem->mutex->owner != RT_NULL || sem->mutex->value != 1) {
            posix_sem_unlock();
            rt_set_errno(EBUSY);
            return -1;
        }
    } else 
#endif
    {
        if(rt_list_len(&sem->sem->parent.suspend_thread) != 0) {
            posix_sem_unlock();
            rt_set_errno(EBUSY);
            return -1;
        }
    }

    /* destroy an unamed posix semaphore */
    posix_sem_delete(sem);
    posix_sem_unlock();
    return 0;
#elif defined(CONFIG_FREERTOS_ENABLE)

    if (!sem) { 
        errno = EINVAL;
        return -1;
    }

    vSemaphoreDelete(SEM_GET_HANDLE(sem));
#if defined(CONFIG_LIBC_SEMAPHORE_FREERTOS_DYNAMIC)
    sem->handle = NULL;
#else
    memset(&sem->handle, 0, sizeof(StaticSemaphore_t));
#endif

    return 0;

#else

    return -1;
#endif
}
