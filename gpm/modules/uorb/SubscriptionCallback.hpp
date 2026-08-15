#ifndef SUBSCRIPTION_CALLBACK_CPP_H_
#define SUBSCRIPTION_CALLBACK_CPP_H_

#include "uorb_common.h"
#include "uorb_device_node.h"
#include "uorb_manager.h"

#include "workqueue/workitem.h"
#include <SubscriptionInterval.hpp>

#include "container/intrusive_node.h"

extern "C" {

}

namespace uORB
{

// Subscription wrapper class with callbacks on new publications
class SubscriptionCallback : public SubscriptionInterval
{
public:
	/**
	 * Constructor
	 *
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
	};

	bool registerCallback()
	{
		if (_subscription.get_node() && uorb_device_node_register_callback(_subscription.get_node(), &_callbacknode)) {
			// registered
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

	virtual void call() = 0;

protected:

	bool _registered{false};

    struct __subscription_callback _callbacknode;

private:
    static void static_call(struct __subscription_callback *node) {
        SubscriptionCallback *self = container_of(node, SubscriptionCallback, _callbacknode);
        self->call();
    }
};

// Subscription with callback that schedules a WorkItem
class SubscriptionCallbackWorkItem : public SubscriptionCallback
{
public:
	/**
	 * Constructor
	 *
	 * @param work_item The WorkItem that will be scheduled immediately on new publications.
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param instance The instance for multi sub.
	 */
	SubscriptionCallbackWorkItem(struct __workitem *work_item, const orb_metadata *meta, uint8_t instance = 0) :
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
			if (updated()) {
                workitem_schedule_now(_work_item);
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
    struct __workitem *_work_item;

	uint8_t _required_updates{0};
};

} // namespace uORB

#endif
