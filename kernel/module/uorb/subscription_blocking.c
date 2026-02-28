#include "subscription_blocking.h"

void subscription_blocking_init(struct __subscription_blocking *sublk, const struct orb_metadata *meta, uint32_t interval_us, uint8_t instance)
{
	pthread_mutexattr_t attr;
	int ret = pthread_mutexattr_init(&attr);

#if defined(PTHREAD_PRIO_NONE)
	ret = pthread_mutexattr_settype(&attr, PTHREAD_PRIO_NONE);
#endif

	ret = pthread_mutex_init(&sublk->_mutex, &attr);
    subscription_callback_init(&sublk->_sub_callback, meta, interval_us, instance);
}

void subscription_blocking_deinit(struct __subscription_blocking *sublk)
{
	pthread_mutex_destroy(&sublk->_mutex);
	pthread_cond_destroy(&sublk->_cv);
}

bool subscription_updated_blocking(struct __subscription_blocking *sublk, uint32_t timeout_us)
{
    if (!sublk->_sub_callback._registered) {
        subscription_callback_register(&sublk->_sub_callback);
    }

    if (subscription_updated(&sublk->_sub_callback._subscription_itv._sub)) {
        // return immediately if updated
        return true;

    } else {
        // otherwise wait

        pthread_mutex_lock(&sublk->_mutex);

        if (timeout_us == 0) {
            // wait with no timeout
            if (pthread_cond_wait(&sublk->_cv, &sublk->_mutex) == 0) {
                pthread_mutex_unlock(&sublk->_mutex);
                return subscription_updated(&sublk->_sub_callback._subscription_itv._sub);
            }

        } else {
            // otherwise wait with timeout based on interval

            // Calculate an absolute time in the future
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            uint64_t nsecs = ts.tv_nsec + (timeout_us * 1000);
            const unsigned billion = (1000 * 1000 * 1000);
            ts.tv_sec += nsecs / billion;
            nsecs -= (nsecs / billion) * billion;
            ts.tv_nsec = nsecs;

            if (pthread_cond_timedwait(&sublk->_cv, &sublk->_mutex, &ts) == 0) {
                pthread_mutex_unlock(&sublk->_mutex);
                return subscription_updated(&sublk->_sub_callback._subscription_itv._sub);
            }
        }

        pthread_mutex_unlock(&sublk->_mutex);
    }

    return false;
}

bool subscription_update_blocking2(struct __subscription_blocking *sublk, void *data, uint32_t timeout_us)
{
    if (subscription_updated_blocking(sublk, timeout_us)) {
        return subscription_copy(&sublk->_sub_callback._subscription_itv._sub, data);
    }

    return false;
}

