#include <semaphore.h>
#include <fcntl.h>
#include "prv_sem.h"

sem_t *sem_open(const char *name, int oflag, ...)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    sem_t* sem;
    va_list arg;
    mode_t mode;
    unsigned int value;

    sem = RT_NULL;

    posix_sem_lock();
    if (oflag & O_CREAT) {
        va_start(arg, oflag);
        mode = (mode_t)va_arg(arg, unsigned int); mode = mode;
        value = va_arg(arg, unsigned int);
        va_end(arg);

        if (oflag & O_EXCL) {
            if (posix_sem_find(name) != RT_NULL) {
                rt_set_errno(EEXIST);
                goto __return;
            }
        }

        sem = (sem_t*) rt_malloc(sizeof(struct posix_sem));
        if (sem == RT_NULL) {
            rt_set_errno(ENFILE);
            goto __return;
        }

        /* create RT-Thread semaphore */
        sem->sem = rt_sem_create(name, value, RT_IPC_FLAG_FIFO);
        if (sem->sem == RT_NULL) {
            rt_set_errno(ENFILE);
            goto __return;
        }

        /* initialize reference count */
        sem->refcount = 1;
        sem->unlinked = 0;
        sem->unamed = 0;

        /* insert semaphore to posix semaphore list */
        posix_sem_insert(sem);
    } else {
        /* find semaphore */
        sem = posix_sem_find(name);
        if (sem != RT_NULL) {
            sem->refcount++;
        } else {
            rt_set_errno(ENOENT);
            goto __return;
        }
    }
    posix_sem_unlock();

    return sem;

__return:
    /* release lock */
    posix_sem_unlock();

    /* release allocated memory */
    if (sem != RT_NULL) {
        /* delete RT-Thread semaphore */
        if (sem->sem != RT_NULL) {
            rt_sem_delete(sem->sem);
        }
        rt_free(sem);
    }

    return RT_NULL;
#else

    return NULL;
#endif
}
