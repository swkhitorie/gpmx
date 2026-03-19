#include <semaphore.h>

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

    result = rt_sem_release(sem->sem);
    if (result == RT_EOK) {
        return 0;
    }

    rt_set_errno(EINVAL);
    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    sem_t *p = (sem_t *)sem;
    int pre_val = Atomic_Increment_u32((uint32_t *)&p->val);
    if (pre_val < 0) {
        xSemaphoreGive((SemaphoreHandle_t)&p->sem);
    }

    return 0;
#else

    return -1;
#endif
}
