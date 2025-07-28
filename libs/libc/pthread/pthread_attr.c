#include "pthread.h"
#include "errno.h"

#include "FreeRTOS.h"
#include "portmacro.h"

#define DEFAULT_STACK_SIZE  (configMINIMAL_STACK_SIZE*sizeof(StackType_t))
#define DEFAULT_PRIORITY    (tskIDLE_PRIORITY)

int pthread_attr_init(pthread_attr_t *attr)
{
    if (attr == NULL) return 0;
    
    attr->stackaddr = 0;
    attr->stacksize = DEFAULT_STACK_SIZE;
    attr->inheritsched = PTHREAD_INHERIT_SCHED;
    attr->schedpolicy = SCHED_FIFO;
    attr->schedparam.sched_priority = DEFAULT_PRIORITY;
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (attr == NULL) return 0;

    memset(attr, 0, sizeof(pthread_attr_t));
    
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int state)
{
    if (attr == NULL) return 0;

    if (state != PTHREAD_CREATE_JOINABLE && state != PTHREAD_CREATE_DETACHED)
        return EINVAL;

    attr->detachstate = state;
    return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state)
{
    if (attr == NULL) return 0;

    *state = (int)attr->detachstate;

    return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    if (attr == NULL) return 0;

    attr->schedpolicy = policy;

    return 0;
}

int pthread_attr_getschedpolicy(pthread_attr_t const *attr, int *policy)
{
    if (attr == NULL) return 0;

    *policy = (int)attr->schedpolicy;

    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr, struct sched_param const *param)
{
    if (attr == NULL || param == NULL) return 0;

    attr->schedparam.sched_priority = param->sched_priority;

    return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
    if (attr == NULL || param == NULL) return 0;

    param->sched_priority = attr->schedparam.sched_priority;

    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stack_size)
{
    if (attr == NULL) return 0;

    attr->stacksize = stack_size;

    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stack_size)
{
    if (attr == NULL) return 0;

    *stack_size = attr->stacksize;

    return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack_addr)
{
    if (attr == NULL) return 0;
    return EOPNOTSUPP;
}

int pthread_attr_getstackaddr(pthread_attr_t const *attr, void **stack_addr)
{
    if (attr == NULL) return 0;
    return EOPNOTSUPP;
}

int pthread_attr_setstack(pthread_attr_t *attr, void *stack_base, size_t stack_size)
{
    if (attr == NULL) return 0;

    attr->stackaddr = stack_base;
    attr->stacksize = stack_size;

    return 0;
}

int pthread_attr_getstack(pthread_attr_t const *attr, void **stack_base, size_t *stack_size)
{
    if (attr == NULL) return 0;

    *stack_base = attr->stackaddr;
    *stack_size = attr->stacksize;

    return 0;
}

int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guard_size)
{
    return EOPNOTSUPP;
}

int pthread_attr_getguardsize(pthread_attr_t const *attr, size_t *guard_size)
{
    return EOPNOTSUPP;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
    if (attr == NULL) return 0;

    attr->inheritsched = inheritsched;

    return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    if (attr == NULL) return 0;

    *inheritsched = attr->inheritsched;

    return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    return EOPNOTSUPP;
}

int pthread_attr_getscope(pthread_attr_t const *attr, int *scope)
{
    return EOPNOTSUPP;
}
