#include "errno.h"
#include "semaphore.h"
#include "utils.h"

int sem_getvalue(sem_t *sem, int *sval)
{
    sem_t *p = (sem_t *)sem;
    *sval = p->val;
    return 0;
}
