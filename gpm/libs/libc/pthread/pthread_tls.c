
#include <pthread.h>
#include "prv_pthread.h"
#include <errno.h>
#include <gpmx/config.h>

#if defined(CONFIG_RTTNANO_ENABLE)
_pthread_key_data_t _thread_keys[PTHREAD_KEY_MAX];

/* initialize key area */
static int pthread_key_system_init(void)
{
    rt_memset(&_thread_keys[0], 0, sizeof(_thread_keys));
    return 0;
}
#endif

void *pthread_getspecific(pthread_key_t key)
{
#if defined(CONFIG_RTTNANO_ENABLE)
    struct _pthread_data* ptd;

    if (rt_thread_self() == NULL) {
        return NULL;
    }

    ptd = (_pthread_data_t *)rt_thread_self()->user_data;

    if (ptd->tls == NULL) {
        return NULL;
    }

    if ((key < PTHREAD_KEY_MAX) && (_thread_keys[key].is_used)) {
        return ptd->tls[key];
    }

    return NULL;
#else
    return NULL;
#endif
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
#if defined(CONFIG_RTTNANO_ENABLE)
    struct _pthread_data* ptd;

    if (rt_thread_self() == NULL) {
        return EINVAL;
    }

    ptd = (_pthread_data_t *)rt_thread_self()->user_data;

    if (ptd->tls == NULL) {
        ptd->tls = (void**)rt_malloc(sizeof(void*) * PTHREAD_KEY_MAX);
    }

    if ((key < PTHREAD_KEY_MAX) && _thread_keys[key].is_used) {
        ptd->tls[key] = (void *)value;
        return 0;
    }

    return EINVAL;
#else
    return ENOTSUP;
#endif
}

int pthread_key_create(pthread_key_t *key, void (*destructor)(void*))
{
#if defined(CONFIG_RTTNANO_ENABLE)
    rt_uint32_t index;

    rt_enter_critical();
    for (index = 0; index < PTHREAD_KEY_MAX; index ++) {

        if (_thread_keys[index].is_used == 0) {
            _thread_keys[index].is_used = 1;
            _thread_keys[index].destructor = destructor;
            *key = index;
            rt_exit_critical();
            return 0;
        }
    }

    rt_exit_critical();
    return EAGAIN;
#else
    return ENOTSUP;
#endif
}

int pthread_key_delete(pthread_key_t key)
{
#if defined(CONFIG_RTTNANO_ENABLE)
    if (key >= PTHREAD_KEY_MAX) {
        return EINVAL;
    }

    rt_enter_critical();
    _thread_keys[key].is_used = 0;
    _thread_keys[key].destructor = 0;
    rt_exit_critical();

    return 0;
#else
    return ENOTSUP;
#endif
}


