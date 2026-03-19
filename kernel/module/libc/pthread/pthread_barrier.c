#include <pthread.h>
#include <errno.h>

int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
    if (!attr) {
        return EINVAL;
    }

    return 0;
}

int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
    if (!attr) {
        return EINVAL;
    }

    *attr = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr,
                                   int                         *pshared)
{
    if (!attr) {
        return EINVAL;
    }

    *pshared = (int)*attr;
    return 0;
}

int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
    if (!attr) {
        return EINVAL;
    }

    if (pshared == PTHREAD_PROCESS_PRIVATE) {
        *attr = PTHREAD_PROCESS_PRIVATE;
        return 0;
    }

    return EINVAL;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    int ret;

    if (!barrier) {
        return EINVAL;
    }

    ret = pthread_mutex_lock(&(barrier->mutex));
    if (ret != 0) {
        return ret;
    }

    if (barrier->count != 0) {
        pthread_mutex_unlock(&(barrier->mutex));
        return EBUSY;
    }

    ret = pthread_mutex_unlock(&(barrier->mutex));
    if (ret != 0) {
        return ret;
    }

    ret = pthread_mutex_destroy(&(barrier->mutex));
    if (ret != 0) {
        return ret;
    }

    ret = pthread_cond_destroy(&(barrier->cond));
    return ret;
}

int pthread_barrier_init(pthread_barrier_t *barrier, 
   const pthread_barrierattr_t *attr, unsigned count)
{
    if (!barrier) {
        return EINVAL;
    }

    if (attr && (*attr != PTHREAD_PROCESS_PRIVATE)) {
        return EINVAL;
    }

    if (count == 0) {
        return EINVAL;
    }

    barrier->count = count;
    pthread_cond_init(&(barrier->cond), NULL);
    pthread_mutex_init(&(barrier->mutex), NULL);
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    int ret;
    if (!barrier) {
        return EINVAL;
    }

    ret = pthread_mutex_lock(&(barrier->mutex));
    if (ret != 0) {
        return EINVAL;
    }

    if (barrier->count == 0) {
        ret = EINVAL;
    } else {
        barrier->count -= 1;
        if (barrier->count == 0) {
            pthread_cond_broadcast(&(barrier->cond));
        } else {
            pthread_cond_wait(&(barrier->cond), &(barrier->mutex));
        }
    }

    pthread_mutex_unlock(&(barrier->mutex));
    return ret;
}
