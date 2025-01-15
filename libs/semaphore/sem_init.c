#include "errno.h"
#include "semaphore.h"
#include "utils.h"

int sem_init(sem_t *sem, int pshared, unsigned value)
{
    int ret = 0;
    sem_t *p = (sem_t *)sem;
    (void)pshared;

    if (value > SEM_VALUE_MAX) {
        // errno = EINVAL;
        ret = -1;
    }
    p->val = (int)value;
    if (ret == 0) {
        (void)xSemaphoreCreateCountingStatic(SEM_VALUE_MAX, 0, &p->sem);
    }

    return ret;
}
