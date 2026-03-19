#include <semaphore.h>
#include "prv_sem.h"

int sem_destroy(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    if ((!sem) || !(sem->unamed)) {
        rt_set_errno(EINVAL);
        return -1;
    }

    posix_sem_lock();
    if(rt_list_len(&sem->sem->parent.suspend_thread) != 0) {
        rt_sem_release(&posix_sem_lock);
        rt_set_errno(EBUSY);
        return -1;
    }

    /* destroy an unamed posix semaphore */
    posix_sem_delete(sem);
    posix_sem_unlock();
    return 0;
#elif defined(CONFIG_FREERTOS_ENABLE)

    sem_t *p = (sem_t *)sem;
    vSemaphoreDelete((SemaphoreHandle_t)&p->sem);
    return 0;
#else

    return -1;
#endif
}
