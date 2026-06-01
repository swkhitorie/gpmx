#ifndef SUBSCRIPTION_INTERVAL_CPP_H_
#define SUBSCRIPTION_INTERVAL_CPP_H_

#include "uorb_common.h"
#include "uorb_manager.h"

#include "Subscription.hpp"

namespace uORB
{

// Base subscription wrapper class
class SubscriptionInterval
{
public:

	/**
	 * Constructor
	 *
	 * @param id The uORB ORB_ID enum for the topic.
	 * @param interval The requested maximum update interval in microseconds.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionInterval(enum ORB_ID id, uint32_t interval_us = 0, uint8_t instance = 0) :
		_subscription{id, instance},
		_interval_us(interval_us)
	{}

	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param interval The requested maximum update interval in microseconds.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionInterval(const orb_metadata *meta, uint32_t interval_us = 0, uint8_t instance = 0) :
		_subscription{meta, instance},
		_interval_us(interval_us)
	{}

	SubscriptionInterval() : _subscription{nullptr} {}

	~SubscriptionInterval() = default;

	bool subscribe() { return _subscription.subscribe(); }

	bool advertised() { return _subscription.advertised(); }

	/**
	 * Check if there is a new update.
	 * */
	bool updated()
	{
		if (advertised() && (hrt_elapsed_time(&_last_update) >= _interval_us)) {
			return _subscription.updated();
		}

		return false;
	}

	/**
	 * Copy the struct if updated.
	 * @param dst The destination pointer where the struct will be copied.
	 * @return true only if topic was updated and copied successfully.
	 */
	bool update(void *dst)
	{
		if (updated()) {
			return copy(dst);
		}

		return false;
	}

	/**
	 * Copy the struct
	 * @param dst The destination pointer where the struct will be copied.
	 * @return true only if topic was copied successfully.
	 */
	bool copy(void *dst)
	{
		if (_subscription.copy(dst)) {
			const hrt_abstime now = hrt_absolute_time();
			// shift last update time forward, but don't let it get further behind than the interval
            uint64_t val = _last_update + _interval_us;
            uint64_t min = now - _interval_us, max = now;
			_last_update = (val < min) ? min : ((val > max) ? max : val);
			return true;
		}

		return false;
	}

	bool		valid() const { return _subscription.valid(); }

	uint8_t		get_instance() const { return _subscription.get_instance(); }
	uint32_t        get_interval_us() const { return _interval_us; }
	unsigned	get_last_generation() const { return _subscription.get_last_generation(); }
	ORB_PRIO	get_priority() { return _subscription.get_priority(); }
	orb_id_t	get_topic() const { return _subscription.get_topic(); }

	/**
	 * Set the interval in microseconds
	 * @param interval The interval in microseconds.
	 */
	void		set_interval_us(uint32_t interval) { _interval_us = interval; }

	/**
	 * Set the interval in milliseconds
	 * @param interval The interval in milliseconds.
	 */
	void		set_interval_ms(uint32_t interval) { _interval_us = interval * 1000; }

protected:

	Subscription	_subscription;
	uint64_t	_last_update{0};	// last update in microseconds
	uint32_t	_interval_us{0};	// maximum update interval in microseconds

};

} // namespace uORB

#endif
