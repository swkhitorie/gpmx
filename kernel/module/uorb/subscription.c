#include "subscription.h"

void subscription_init(struct __subscription *sub, enum ORB_ID id, uint8_t instance)
{
    sub->_orb_id = id;
    sub->_instance = instance;
}

void subscription_init2(struct __subscription *sub, const struct orb_metadata *meta, uint8_t instance)
{
    if (meta == NULL) {
        sub->_orb_id = ORB_ID_INVALID;
    } else {
        sub->_orb_id = (enum ORB_ID)(meta->o_id);
    }

    sub->_instance = instance;
}

void subscription_deinit(struct __subscription *sub)
{
    subscription_unsubscribe(sub);
}

bool subscription_subscribe(struct __subscription *sub)
{
    if (sub->_node != NULL) {
        return true;
    }

    if (sub->_orb_id != ORB_ID_INVALID) {

        struct __uorb_device_master *device_master = uorb_manager_get_device_master(uorb_manager_instance());

        if (device_master != NULL) {

            if (!uorb_device_master_node_exists(device_master, sub->_orb_id, sub->_instance)) {
                return false;
            }

            struct __uorb_device_node *node = uorb_device_master_get_node2(device_master, subscription_get_topic(sub), sub->_instance);

            if (node != NULL) {
                sub->_node = node;
                uorb_device_node_add_internal_subscriber(sub->_node);

                // If there were any previous publications, allow the subscriber to read them
                const unsigned curr_gen = uorb_device_node_published_message_count(sub->_node);
                const uint8_t q_size = uorb_device_node_get_queue_size(sub->_node);

                if (q_size < curr_gen) {
                    sub->_last_generation = curr_gen - q_size;

                } else {
                    sub->_last_generation = 0;
                }

                return true;
            }
        }
    }

    return false;
}

void subscription_unsubscribe(struct __subscription *sub)
{
	if (sub->_node != NULL) {
		uorb_device_node_remove_internal_subscriber(sub->_node);
	}

	sub->_node = NULL;
	sub->_last_generation = 0;
}

bool subscription_valid(struct __subscription *sub)
{
    return sub->_node != NULL;
}

bool subscription_advertised(struct __subscription *sub)
{
    if (subscription_valid(sub)) {
        return uorb_device_node_is_advertised(sub->_node);
    }

    // try to initialize
    if (subscription_subscribe(sub)) {
        // check again if valid
        if (subscription_valid(sub)) {
            return uorb_device_node_is_advertised(sub->_node);
        }
    }

    return false;
}

bool subscription_updated(struct __subscription *sub)
{
    if (subscription_advertised(sub)) {
        return uorb_device_node_published_message_count(sub->_node) != sub->_last_generation;
    }
    return false; 
}

bool subscription_update(struct __subscription *sub, void *dst)
{
    if (subscription_updated(sub)) {
        return subscription_copy(sub, dst);
    }

    return false;
}

bool subscription_copy(struct __subscription *sub, void *dst)
{
    if (subscription_advertised(sub)) {
        return uorb_device_node_copy(sub->_node, dst, &sub->_last_generation);
    }

    return false;
}

uint8_t  subscription_get_instance(struct __subscription *sub)
{
    return sub->_instance;
}

uint32_t subscription_get_last_generation(struct __subscription *sub)
{
    return sub->_last_generation;
}

enum ORB_PRIO subscription_get_priority(struct __subscription *sub)
{
    if (subscription_advertised(sub)) {
        return uorb_device_node_get_priority(sub->_node);
    }

    return ORB_PRIO_UNINITIALIZED;
}

orb_id_t subscription_get_topic(struct __subscription *sub)
{
    return get_orb_meta(sub->_orb_id);
}

struct __uorb_device_node *subscription_get_node(struct __subscription *sub)
{
    return sub->_node;
}
