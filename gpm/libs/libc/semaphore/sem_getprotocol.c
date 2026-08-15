#include <semaphore.h>
#include <gpmx/config.h>

int sem_getprotocol(sem_t *sem, int protocol)
{
    (void)sem;
    (void)protocol;
    return 0;
}
