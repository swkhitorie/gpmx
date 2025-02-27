#include "errno.h"
#include "semaphore.h"
#include "utils.h"

int sem_trywait(sem_t *sem)
{
    int ret = 0;

    struct timespec xTimeout = { 0 };

    ret = sem_timedwait( sem, &xTimeout );

    //if ((ret == -1 ) && errno == ETIMEDOUT) {
        // errno = EAGAIN;
    //}
    return ret;
}
