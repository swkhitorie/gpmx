#include <semaphore.h>
#include <errno.h>
#include "prv_sem.h"

int sem_init(sem_t *sem, int pshared, unsigned value)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    char name[RT_NAME_MAX];
    static rt_uint16_t psem_number = 0;

    if (sem == RT_NULL) {
        rt_set_errno(EINVAL);
        return -1;
    }

    rt_snprintf(name, sizeof(name), "psem%02d", psem_number++);
    sem->sem = rt_sem_create(name, value, RT_IPC_FLAG_FIFO);
    if (sem->sem == RT_NULL) {
        rt_set_errno(ENOMEM);
        return -1;
    }

    /* initialize posix semaphore */
    sem->refcount = 1;
    sem->unlinked = 0;
    sem->unamed = 1;

    posix_sem_lock();
    posix_sem_insert(sem);
    posix_sem_unlock();
#elif defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    sem_t *p = (sem_t *)sem;
    (void)pshared;
    if (value > SEM_VALUE_MAX) {
        errno = EINVAL;
        ret = -1;
    }
    p->val = (int)value;
    if (ret == 0) {
        xSemaphoreCreateCountingStatic(SEM_VALUE_MAX, 0, &p->sem);
    }

    return ret;
#else

    return -1;
#endif
}

