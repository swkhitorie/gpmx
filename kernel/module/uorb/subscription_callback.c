#include "subscription_callback.h"

void subscription_callback_init(struct __subscription_callback *sub_cal, const struct orb_metadata *meta, uint32_t interval_us, uint8_t instance)
{
    subscription_interval_init2(&sub_cal->_subscription_itv, meta, interval_us, instance);
}

void subscription_callback_deinit(struct __subscription_callback *sub_cal)
{
    subscription_callback_unregister(sub_cal);
}

bool subscription_callback_register(struct __subscription_callback *sub_cal)
{
    if (subscription_get_node(&sub_cal->_subscription_itv._sub) && 
        uorb_device_node_register_callback(subscription_get_node(&sub_cal->_subscription_itv._sub), sub_cal)) {
        // registered
        sub_cal->_registered = true;

    } else {
        // force topic creation by subscribing with old API
        int fd = orb_subscribe_multi(
            subscription_get_topic(&sub_cal->_subscription_itv._sub), 
            subscription_get_instance(&sub_cal->_subscription_itv._sub));

        // try to register callback again
        if (subscription_subscribe(&sub_cal->_subscription_itv._sub)) {
            if (subscription_get_node(&sub_cal->_subscription_itv._sub) && 
                uorb_device_node_register_callback(subscription_get_node(&sub_cal->_subscription_itv._sub), sub_cal)) {
                sub_cal->_registered = true;
            }
        }

        orb_unsubscribe(fd);
    }

    return sub_cal->_registered;
}

void subscription_callback_unregister(struct __subscription_callback *sub_cal)
{
    if (subscription_get_node(&sub_cal->_subscription_itv._sub)) {
        uorb_device_node_unregister_callback(subscription_get_node(&sub_cal->_subscription_itv._sub), sub_cal);
    }

    sub_cal->_registered = false;
}

static void subscription_callback_workitem_call(struct __subscription_callback *sub_cal)
{
    struct __subscription_callback_workitem *sub_cal_witem = NULL;
    sub_cal_witem = container_of(sub_cal, struct __subscription_callback_workitem, _sub_callback);

    // schedule immediately if updated (queue depth or subscription interval)
    if ((sub_cal_witem->_required_updates == 0)
        || (uorb_device_node_published_message_count(subscription_get_node(&sub_cal->_subscription_itv._sub)) 
            >= (subscription_get_last_generation(&sub_cal->_subscription_itv._sub) + sub_cal_witem->_required_updates))) {
        if (subscription_updated(&sub_cal->_subscription_itv._sub)) {
            workitem_schedule_now(sub_cal_witem->_work_item);
        }
    }
}

void subscription_callback_workitem_init(struct __subscription_callback_workitem *sub_cal_witem, struct __workitem *work_item, 
    const struct orb_metadata *meta, uint8_t instance)
{
    subscription_callback_init(&sub_cal_witem->_sub_callback, meta, 0, instance);
    sub_cal_witem->_work_item = work_item;
    sub_cal_witem->_sub_callback.call = subscription_callback_workitem_call;
}

void subscription_callback_workitem_set_required_updates(struct __subscription_callback_workitem *sub_cal_witem, uint8_t required_updates)
{
    sub_cal_witem->_required_updates = required_updates;
}
