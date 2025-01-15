#include "errno.h"
#include "semaphore.h"
#include "utils.h"

int sem_destroy(sem_t *sem)
{
    sem_t *p = (sem_t *)sem;

    vSemaphoreDelete((SemaphoreHandle_t)&p->sem);
    return 0;
}
