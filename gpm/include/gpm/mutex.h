#ifndef __INCLUDE_KMUTEX_H
#define __INCLUDE_KMUTEX_H

#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define NXRMUTEX_NO_HOLDER     (0)

typedef sem_t mutex_t;

struct rmutex_s
{
    mutex_t mutex;
    pthread_t holder;
    uint16_t count;
};

typedef struct rmutex_s rmutex_t;

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

static inline int kmutex_init(mutex_t *mutex)
{
    int ret = sem_init(mutex, 0, 1);
    if (ret != 0) {
        return -errno;
    }
    return 0;
}

static inline int kmutex_destroy(mutex_t *mutex)
{
    int ret = sem_destroy(mutex);
    if (ret != 0) {
        return -errno;
    }
    return 0;
}

static inline int kmutex_lock(mutex_t *mutex)
{
    int ret;

    while ((ret = sem_wait(mutex)) != 0) {
        if (errno != EINTR && errno != ECANCELED) {
            return -errno;
        }
    }

    return 0;
}

static inline int kmutex_trylock(mutex_t *mutex)
{
    int ret = sem_trywait(mutex);
    if (ret != 0) {
        return -errno;
    }
    return 0;
}

static inline bool kmutex_is_locked(mutex_t *mutex)
{
    int val;
    int ret = sem_getvalue(mutex, &val);
    return val < 1;
}

static inline int kmutex_unlock(mutex_t *mutex)
{
    int ret = sem_post(mutex);
    if (ret != 0) {
        return -errno;
    }
    return 0;
}

static inline int krmutex_init(rmutex_t *rmutex)
{
    rmutex->count = 0;
    rmutex->holder = NXRMUTEX_NO_HOLDER;
    return kmutex_init(&rmutex->mutex);
}

static inline int krmutex_destroy(rmutex_t *rmutex)
{
    return kmutex_destroy(&rmutex->mutex);
}

static inline int krmutex_lock(rmutex_t *rmutex)
{
    pthread_t tid = pthread_self();
    int ret;

    if (rmutex->holder == tid) {
        rmutex->count++;
        ret = 0;
    } else {
        ret = kmutex_lock(&rmutex->mutex);
        if (ret == 0) {
            rmutex->holder = tid;
            rmutex->count = 1;
        }
    }

    return ret;
}

static inline int krmutex_trylock( rmutex_t *rmutex)
{
    pthread_t tid = pthread_self();
    int ret;

    if (rmutex->holder == tid) {
        rmutex->count++;
        ret = 0;
    } else {
        ret = kmutex_trylock(&rmutex->mutex);
        if (ret == 0) {
            rmutex->holder = tid;
            rmutex->count = 1;
        }
    }

    return ret;
}

static inline bool krmutex_is_locked(rmutex_t *rmutex)
{
    return rmutex->count > 0;
}

static inline bool krmutex_is_hold(rmutex_t *rmutex)
{
    return rmutex->holder == pthread_self();
}

static inline int krmutex_unlock(rmutex_t *rmutex)
{
    pthread_t tid = pthread_self();
    int ret = 0;

    if (rmutex->count == 1) {
        rmutex->count = 0;
        rmutex->holder = NXRMUTEX_NO_HOLDER;
        ret = kmutex_unlock(&rmutex->mutex);
    } else {
        rmutex->count--;
    }

    return ret;
}

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_NUTTX_MUTEX_H */
