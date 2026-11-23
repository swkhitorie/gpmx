#ifndef SUBSCRIPTION_MULTIARRAY_CPP_H_
#define SUBSCRIPTION_MULTIARRAY_CPP_H_

#include "uorb_common.h"
#include "uorb_manager.h"

#include "Subscription.hpp"

namespace uORB
{

/**
 * An array of uORB::Subscriptions of the same topic
 */
template<typename T, uint8_t SIZE = ORB_MULTI_MAX_INSTANCES>
class SubscriptionMultiArray
{
public:
	// static_assert(SIZE <= ORB_MULTI_MAX_INSTANCES, "size must be <= uORB max instances");

	static constexpr uint8_t size() { return SIZE; }

	/**
	 * Constructor
	 *
	 * @param id The uORB ORB_ID enum for the topic.
	 */
	explicit SubscriptionMultiArray(enum ORB_ID id)
	{
		for (uint8_t i = 0; i < SIZE; i++) {
			_subscriptions[i] = Subscription{id, i};
			_subscriptions[i].subscribe();
		}
	}

    explicit SubscriptionMultiArray(const orb_metadata *meta)
    {
        for (uint8_t i = 0; i < SIZE; i++) {
            _subscriptions[i] = Subscription{meta, i};
            _subscriptions[i].subscribe();
        }
    }

	~SubscriptionMultiArray() = default;

	Subscription &operator [](int i) { return _subscriptions[i]; }
	const Subscription &operator [](int i) const { return _subscriptions[i]; }

	Subscription *begin() { return _subscriptions; }
	Subscription *end() { return _subscriptions + SIZE; }

	const Subscription *begin() const { return _subscriptions; }
	const Subscription *end() const { return _subscriptions + SIZE; }

	// true if any instance is advertised
	bool advertised()
	{
		for (auto &s : _subscriptions) {
			if (s.advertised()) {
				return true;
			}
		}

		return false;
	}

	// return the number of instances currently advertised
	uint8_t advertised_count()
	{
		uint8_t count = 0;

		for (auto &s : _subscriptions) {
			if (s.advertised()) {
				count++;
			}
		}

		return count;
	}

	// true if any instance is updated
	bool updated()
	{
		for (auto &s : _subscriptions) {
			if (s.updated()) {
				return true;
			}
		}

		return false;
	}

private:
	Subscription _subscriptions[SIZE];
};

} // namespace uORB

#endif
