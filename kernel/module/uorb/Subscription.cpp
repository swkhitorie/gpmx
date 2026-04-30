#include "Subscription.hpp"

namespace uORB
{

bool Subscription::subscribe()
{
	// check if already subscribed
	if (_node != nullptr) {
		return true;
	}

	if (_orb_id != ORB_ID_INVALID) {

        struct __uorb_device_master *device_master = uorb_manager_get_device_master(uorb_manager_instance());

		if (device_master != nullptr) {

			if (!uorb_device_master_node_exists(device_master, _orb_id, _instance)) {
				return false;
			}

            struct __uorb_device_node *node = uorb_device_master_get_node2(device_master, get_topic(), _instance);

			if (node != nullptr) {
				_node = node;
                uorb_device_node_add_internal_subscriber(_node);

				// If there were any previous publications, allow the subscriber to read them
				const unsigned curr_gen =  uorb_device_node_published_message_count(_node);
				const uint8_t q_size = uorb_device_node_get_queue_size(_node);

				if (q_size < curr_gen) {
					_last_generation = curr_gen - q_size;

				} else {
					_last_generation = 0;
				}

				return true;
			}
		}
	}

	return false;
}

void Subscription::unsubscribe()
{
	if (_node != nullptr) {
		uorb_device_node_remove_internal_subscriber(_node);
	}

	_node = nullptr;
	_last_generation = 0;
}

} // namespace uORB
