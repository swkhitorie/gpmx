#ifndef SUBSCRIPTION_CALLBACK_CPP_H_
#define SUBSCRIPTION_CALLBACK_CPP_H_

#include "uorb_common.h"
#include "uorb_device_node.h"
#include "uorb_manager.h"

#include "workqueue/WorkItem.hpp"

#include "SubscriptionInterval.hpp"

#include "container/intrusive_node.h"

namespace uORB
{

#if 0
// Subscription wrapper class with callbacks on new publications
class SubscriptionCallback : public SubscriptionInterval
{
public:
	/**
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param interval_us The requested maximum update interval in microseconds.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionCallback(const orb_metadata *meta, uint32_t interval_us = 0, uint8_t instance = 0) :
		SubscriptionInterval(meta, interval_us, instance)
	{
		_callbacknode.call = &SubscriptionCallback::static_call;
	}

	virtual ~SubscriptionCallback()
	{
		unregisterCallback();
	}

	bool registerCallback()
	{
		if (_subscription.get_node() && uorb_device_node_register_callback(_subscription.get_node(), &_callbacknode)) {
			_registered = true;

		} else {
			// force topic creation by subscribing with old API
			int fd = orb_subscribe_multi(_subscription.get_topic(), _subscription.get_instance());

			// try to register callback again
			if (_subscription.subscribe()) {
				if (_subscription.get_node() && uorb_device_node_register_callback(_subscription.get_node(), &_callbacknode)) {
					_registered = true;
				}
			}

			orb_unsubscribe(fd);
		}

		return _registered;
	}

	void unregisterCallback()
	{
		if (_subscription.get_node()) {
			uorb_device_node_unregister_callback(_subscription.get_node(), &_callbacknode);
		}

		_registered = false;
	}

	/**
	 * Change subscription instance
	 * @param instance The new multi-Subscription instance
	 */
	bool ChangeInstance(uint8_t instance)
	{
		bool ret = false;

		if (instance != get_instance()) {
			const bool registered = _registered;

			if (registered) {
				unregisterCallback();
			}

			if (_subscription.ChangeInstance(instance)) {
				ret = true;
			}

			if (registered) {
				registerCallback();
			}

		} else {
			// already on desired index
			return true;
		}

		return ret;
	}

	virtual void call() = 0;

	bool registered() const { return _registered; }

protected:

	bool _registered{false};

	struct __subscription_callback _callbacknode;

private:
	static void static_call(struct __subscription_callback *node)
	{
		SubscriptionCallback *self = container_of(node, SubscriptionCallback, _callbacknode);
		self->call();
	}
};

#else

class SubscriptionCallback : public SubscriptionInterval
{
public:
	SubscriptionCallback(const orb_metadata *meta, uint32_t interval_us = 0, uint8_t instance = 0) :
		SubscriptionInterval(meta, interval_us, instance)
	{
		_callbacknode.owner = this;
		_callbacknode.call  = &SubscriptionCallback::static_call;
	}

	virtual ~SubscriptionCallback()
	{
		unregisterCallback();
	}

	bool registerCallback()
	{
		if (_subscription.get_node() && uorb_device_node_register_callback(_subscription.get_node(), &_callbacknode)) {
			_registered = true;

		} else {
			int fd = orb_subscribe_multi(_subscription.get_topic(), _subscription.get_instance());

			if (_subscription.subscribe()) {
				if (_subscription.get_node() && uorb_device_node_register_callback(_subscription.get_node(), &_callbacknode)) {
					_registered = true;
				}
			}

			orb_unsubscribe(fd);
		}

		return _registered;
	}

	void unregisterCallback()
	{
		if (_subscription.get_node()) {
			uorb_device_node_unregister_callback(_subscription.get_node(), &_callbacknode);
		}

		_registered = false;
	}

	bool ChangeInstance(uint8_t instance)
	{
		bool ret = false;

		if (instance != get_instance()) {
			const bool registered = _registered;

			if (registered) {
				unregisterCallback();
			}

			if (_subscription.ChangeInstance(instance)) {
				ret = true;
			}

			if (registered) {
				registerCallback();
			}

		} else {
			// already on desired index
			return true;
		}

		return ret;
	}

	virtual void call() = 0;

	bool registered() const { return _registered; }

protected:

	struct CallbackNode : public __subscription_callback {
		SubscriptionCallback *owner{nullptr};
	};

	CallbackNode _callbacknode;

	bool _registered{false};

private:
	static void static_call(struct __subscription_callback *node)
	{
		CallbackNode *self = static_cast<CallbackNode *>(node);
		self->owner->call();
	}
};

#endif

// Subscription with callback that schedules a WorkItem
class SubscriptionCallbackWorkItem : public SubscriptionCallback
{
public:
	/**
	 * @param work_item The WorkItem that will be scheduled immediately on new publications.
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionCallbackWorkItem(px4::WorkItem *work_item, const orb_metadata *meta, uint8_t instance = 0) :
		SubscriptionCallback(meta, 0, instance),	// interval 0
		_work_item(work_item)
	{
	}

	virtual ~SubscriptionCallbackWorkItem() = default;

	void call() override
	{
		// schedule immediately if updated (queue depth or subscription interval)
		if ((_required_updates == 0)
		    || (uorb_device_node_published_message_count(_subscription.get_node()) >=
			(_subscription.get_last_generation() + _required_updates))) {

			if (updated() && (_work_item != nullptr)) {
				_work_item->ScheduleNow();
			}
		}
	}

	/**
	 * Optionally limit callback until more samples are available.
	 *
	 * @param required_updates Number of queued updates required before a callback can be called.
	 */
	void set_required_updates(uint8_t required_updates)
	{
		// TODO: constrain to queue depth?
		_required_updates = required_updates;
	}

private:
	px4::WorkItem *_work_item{nullptr};

	uint8_t _required_updates{0};
};

} // namespace uORB

#endif
