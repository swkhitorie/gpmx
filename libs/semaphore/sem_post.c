#include "errno.h"
#include "semaphore.h"
#include "utils.h"
#include "atomic.h"

int sem_post(sem_t *sem)
{
    sem_t *p = (sem_t *)sem;

    int pre_val = Atomic_Increment_u32((uint32_t *)&p->val);
    if (pre_val < 0) {
        (void)xSemaphoreGive((SemaphoreHandle_t)&p->sem);
    }

    return 0;
}
