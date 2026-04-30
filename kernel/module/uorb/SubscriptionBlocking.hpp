#ifndef SUBSCRIPTION_BLOCKING_CPP_H_
#define SUBSCRIPTION_BLOCKING_CPP_H_

#include "SubscriptionCallback.hpp"

#include "uorb_common.h"
#include "uorb_device_node.h"
#include "uorb_manager.h"

#include "container/intrusive_node.h"
#include <time.h>
#include <pthread.h>

namespace uORB
{

// Subscription with callback that schedules a WorkItem
template<typename T>
class SubscriptionBlocking : public SubscriptionCallback
{
public:
	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param interval_us The requested maximum update interval in microseconds.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionBlocking(const orb_metadata *meta, uint32_t interval_us = 0, uint8_t instance = 0) :
		SubscriptionCallback(meta, interval_us, instance)
	{
		// pthread_mutexattr_init
		pthread_mutexattr_t attr;
		int ret_attr_init = pthread_mutexattr_init(&attr);
		if (ret_attr_init != 0) {
			// error ret_attr_init
		}

#if defined(PTHREAD_PRIO_NONE)
		// pthread_mutexattr_settype
		//  PTHREAD_PRIO_NONE not available on cygwin
		int ret_mutexattr_settype = pthread_mutexattr_settype(&attr, PTHREAD_PRIO_NONE);
		if (ret_mutexattr_settype != 0) {
			// error ret_mutexattr_settype
		}
#endif

		int ret_mutex_init = pthread_mutex_init(&_mutex, &attr);
		if (ret_mutex_init != 0) {
			// error ret_mutex_init
		}
	}

	virtual ~SubscriptionBlocking()
	{
		pthread_mutex_destroy(&_mutex);
		pthread_cond_destroy(&_cv);
	}

	void call() override
	{
		// signal immediately if no interval, otherwise only if interval has elapsed
		if ((_interval_us == 0) || (hrt_elapsed_time(&_last_update) >= _interval_us)) {
			pthread_cond_signal(&_cv);
		}
	}

	/**
	 * Block until updated.
	 * @param timeout_us The requested timeout in microseconds, or 0 to wait indefinitely.
	 *
	 * @return true only if topic was updated
	 */
	bool updatedBlocking(uint32_t timeout_us = 0)
	{
		if (!_registered) {
			registerCallback();
		}

		if (updated()) {
			// return immediately if updated
			return true;

		} else {
			// otherwise wait

            pthread_mutex_lock(&_mutex);

			if (timeout_us == 0) {
				// wait with no timeout
				if (pthread_cond_wait(&_cv, &_mutex) == 0) {
                    bool ret = updated();
                    pthread_mutex_unlock(&_mutex);
					return ret;
				}

			} else {
				// otherwise wait with timeout based on interval

				// Calculate an absolute time in the future
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				uint64_t nsecs = ts.tv_nsec + (timeout_us * 1000);
				static constexpr unsigned billion = (1000 * 1000 * 1000);
				ts.tv_sec += nsecs / billion;
				nsecs -= (nsecs / billion) * billion;
				ts.tv_nsec = nsecs;

				if (pthread_cond_timedwait(&_cv, &_mutex, &ts) == 0) {
                    bool ret = updated();
                    pthread_mutex_unlock(&_mutex);
					return ret;
				}
			}

            pthread_mutex_unlock(&_mutex);
		}

		return false;
	}

	/**
	 * Copy the struct if updated.
	 * @param data The data reference where the struct will be copied.
	 * @param timeout_us The requested timeout in microseconds, or 0 to wait indefinitely.
	 *
	 * @return true only if topic was updated and copied successfully.
	 */
	bool updateBlocking(T &data, uint32_t timeout_us = 0)
	{
		if (updatedBlocking(timeout_us)) {
			return copy(&data);
		}

		return false;
	}

private:

	pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t	_cv = PTHREAD_COND_INITIALIZER;

};

} // namespace uORB

#endif
