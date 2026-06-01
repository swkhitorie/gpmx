#ifndef SUBSCRIPTION_CPP_H_
#define SUBSCRIPTION_CPP_H_

#include <uorb/topics/uorb_topics.h>

#include "uorb_common.h"
#include "uorb_device_node.h"
#include "uorb_manager.h"

namespace uORB
{

class SubscriptionCallback;

class Subscription
{
public:

	/**
	 * Constructor
	 *
	 * @param id The uORB ORB_ID enum for the topic.
	 * @param instance The instance for multi sub.
	 */
	Subscription(enum ORB_ID id, uint8_t instance = 0) :
		_orb_id(id),
		_instance(instance)
	{
	}

	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param instance The instance for multi sub.
	 */
	Subscription(const orb_metadata *meta, uint8_t instance = 0) :
		_orb_id((meta == nullptr) ? ORB_ID_INVALID : static_cast<enum ORB_ID>(meta->o_id)),
		_instance(instance)
	{
	}

	~Subscription()
	{
		unsubscribe();
	}

	bool subscribe();
	void unsubscribe();

	bool valid() const { return _node != nullptr; }
	bool advertised()
	{
		if (valid()) {
			return uorb_device_node_is_advertised(_node);
		}

		// try to initialize
		if (subscribe()) {
			// check again if valid
			if (valid()) {
				return uorb_device_node_is_advertised(_node);
			}
		}

		return false;
	}

	/**
	 * Check if there is a new update.
	 * */
	bool updated() { return advertised() ? (uorb_device_node_published_message_count(_node) != _last_generation) : false; }

	/**
	 * Update the struct
	 * @param dst The uORB message struct we are updating.
	 */
	bool update(void *dst) { return updated() ? copy(dst) : false; }

	/**
	 * Copy the struct
	 * @param dst The uORB message struct we are updating.
	 */
	bool copy(void *dst) { return advertised() ? uorb_device_node_copy(_node, dst, &_last_generation) : false; }

	uint8_t  get_instance() const { return _instance; }
	unsigned get_last_generation() const { return _last_generation; }
	enum ORB_PRIO get_priority() { return advertised() ? uorb_device_node_get_priority(_node): ORB_PRIO_UNINITIALIZED; }
	orb_id_t get_topic() const { return get_orb_meta(_orb_id); }

protected:

	friend class SubscriptionCallback;
	friend class SubscriptionCallbackWorkItem;

	struct __uorb_device_node *get_node() { return _node; }

	struct __uorb_device_node *_node{nullptr};

	uint32_t _last_generation{0}; /**< last generation the subscriber has seen */

	enum ORB_ID _orb_id{ORB_ID_INVALID};
	uint8_t _instance{0};
};

// Subscription wrapper class with data
template<class T>
class SubscriptionData : public Subscription
{
public:
	/**
	 * Constructor
	 *
	 * @param id The uORB metadata ORB_ID enum for the topic.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionData(enum ORB_ID id, uint8_t instance = 0) :
		Subscription(id, instance)
	{
		copy(&_data);
	}

	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionData(const orb_metadata *meta, uint8_t instance = 0) :
		Subscription(meta, instance)
	{
		copy(&_data);
	}

	~SubscriptionData() = default;

	// no copy, assignment, move, move assignment
	SubscriptionData(const SubscriptionData &) = delete;
	SubscriptionData &operator=(const SubscriptionData &) = delete;
	SubscriptionData(SubscriptionData &&) = delete;
	SubscriptionData &operator=(SubscriptionData &&) = delete;

	// update the embedded struct.
	bool update() { return Subscription::update((void *)(&_data)); }

	const T &get() const { return _data; }

private:

	T _data{};
};

} // namespace uORB

#endif
