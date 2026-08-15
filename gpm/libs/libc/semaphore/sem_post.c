#include <semaphore.h>
#include <gpmx/config.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include "atomic.h"
#endif

int sem_post(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t result;
    if (!sem) {
        rt_set_errno(EINVAL);
        return -1;
    }

#if defined(CONFIG_LIBC_SEMAPHORE_INHERIT)
    if (sem->protocol == SEM_PRIO_INHERIT) {
        result = rt_mutex_release(sem->mutex);
    } else
#endif
    {
        result = rt_sem_release(sem->sem);
    }

    if (result != RT_EOK) {
        rt_set_errno(EACCES);
        return -1;
    }

    return 0;
#elif defined(CONFIG_FREERTOS_ENABLE)

    if (!sem) { 
        errno = EINVAL;
        return -1;
    }

#if defined(CONFIG_LIBC_SEMAPHORE_CACHE)
    int pre_val = Atomic_Increment_u32((uint32_t *)&sem->val);
    if (pre_val < 0 || sem->protocol == SEM_PRIO_INHERIT)
#endif
    {
        if (xSemaphoreGive(SEM_GET_HANDLE(sem)) != pdTRUE) {
            errno = EACCES; 
            return -1;
        }
    }
    return 0;

#else

    return -1;
#endif
}
