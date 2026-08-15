#include <semaphore.h>
#include "prv_sem.h"
#include <gpmx/config.h>

int sem_unlink(const char *name)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    sem_t *psem;

    posix_sem_lock();
    psem = posix_sem_find(name);
    if (psem != RT_NULL) {
        psem->unlinked = 1;
        if (psem->refcount == 0) {
            posix_sem_delete(psem);
        }
        posix_sem_unlock();

        return 0;
    }
    posix_sem_unlock();

    /* no this entry */
    rt_set_errno(ENOENT);
    return -1;
#else

    return -1;
#endif
}
