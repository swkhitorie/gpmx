#include "errno.h"
#include "semaphore.h"
#include "utils.h"

int sem_wait(sem_t *sem)
{
    return sem_timedwait(sem, NULL);
}
