#include <semaphore.h>

int sem_getvalue(sem_t *sem, int *sval)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    if (!sem || !sval) {
        rt_set_errno(EINVAL);
        return -1;
    }
    *sval = sem->sem->value;
    return 0;
#elif defined(CONFIG_FREERTOS_ENABLE)

    sem_t *p = (sem_t *)sem;
    *sval = p->val;
    return 0;
#else

    return -1;
#endif
}
