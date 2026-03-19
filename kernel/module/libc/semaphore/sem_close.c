#include <semaphore.h>
#include "prv_sem.h"

int sem_close(sem_t *sem)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    if (sem == RT_NULL) {
        rt_set_errno(EINVAL);
        return -1;
    }

    posix_sem_lock();
    sem->refcount--;
    if (sem->refcount == 0) {
        if (sem->unlinked) {
            posix_sem_delete(sem);
        }
        sem = RT_NULL;
    }
    posix_sem_unlock();
    return 0;
#else

    return -1;
#endif
}