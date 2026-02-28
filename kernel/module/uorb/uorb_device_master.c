#include "uorb_device_master.h"
#include "uorb_manager.h"
#include "uorb_gnode.h"
#include "kmodule_defines.h"
#include <math.h>

#include "FreeRTOS.h"

static void uorb_device_master_lock(struct __uorb_device_master *master)
{
    do {} while (sem_wait(&master->_lock) != 0);
}

static void uorb_device_master_unlock(struct __uorb_device_master *master)
{
    sem_post(&master->_lock);
}

static struct __uorb_device_node *uorb_device_master_get_node_locked(struct __uorb_device_master *master, const struct orb_metadata *meta, const uint8_t instance)
{
    struct intrusive_node *iterator = NULL;
    struct __uorb_device_node *node = NULL;

    for (iterator = master->_node_list.head; iterator != NULL; iterator = iterator->next) {
        node = container_of(iterator, struct __uorb_device_node, sorted_list_node);

        if ((strcmp(uorb_device_node_get_name(node), meta->o_name) == 0) && (uorb_device_node_get_instance(node) == instance)) {
            return node;
        }
    }

	return NULL;
}

int uorb_device_master_add_new_nodes(struct __uorb_device_master *master, struct __device_node_static_data **first_node, int *num_topics, 
                size_t *max_topic_name_length, char **topic_filter, int num_filters)
{
    struct intrusive_node *iterator = NULL;
    struct __uorb_device_node *node = NULL;
    struct __device_node_static_data *cur_node = NULL;
    struct __device_node_static_data *last_node = *first_node;
    struct orb_metadata *meta;

    *num_topics = 0;

	if (last_node) {
		while (last_node->next) {
			last_node = last_node->next;
		}
	}

    for (iterator = master->_node_list.head; iterator != NULL; iterator = iterator->next) {
        node = container_of(iterator, struct __uorb_device_node, sorted_list_node);

        (*num_topics)++;

        //check if already added
        cur_node = *first_node;

        while (cur_node && cur_node->node != node) {
            cur_node = cur_node->next;
        }

        if (cur_node) {
            continue;
        }

		if (num_filters > 0 && topic_filter) {
			bool matched = false;

			for (int i = 0; i < num_filters; ++i) {
                meta = uorb_device_node_get_meta(node);
				if (strstr(meta->o_name, topic_filter[i])) {
					matched = true;
				}
			}

			if (!matched) {
				continue;
			}
		}

		if (last_node) {
			last_node->next = pvPortMalloc(sizeof(struct __device_node_static_data));
			last_node->next->next = NULL;
			last_node->next->last_pub_msg_count = 0;
			last_node->next->node = NULL;
			last_node->next->pub_msg_delta = 0;

			last_node = last_node->next;

		} else {
			*first_node = last_node = pvPortMalloc(sizeof(struct __device_node_static_data));
			last_node->next = NULL;
			last_node->last_pub_msg_count = 0;
			last_node->node = NULL;
			last_node->pub_msg_delta = 0;
		}

		if (!last_node) {
			return -ENOMEM;
		}

		last_node->node = node;

        meta = uorb_device_node_get_meta(last_node->node);
		size_t name_length = strlen(meta->o_name);

		if (name_length > *max_topic_name_length) {
			*max_topic_name_length = name_length;
		}

		last_node->last_pub_msg_count = uorb_device_node_published_message_count(last_node->node);
    }

	return 0;
}

void uorb_device_master_init(struct __uorb_device_master *master)
{
    sem_init(&master->_lock, 0, 1);

	intrusive_sorted_list_init(&master->_node_list, uorb_device_cmp_method);
	for (int i = 0; i < ORB_MULTI_MAX_INSTANCES; i++) {
		node_bits_init(&master->_node_exists[i]);
	}

}

void uorb_device_master_deinit(struct __uorb_device_master *master)
{
    sem_destroy(&master->_lock);
}

int uorb_device_master_advertise(struct __uorb_device_master *master, struct orb_metadata *meta, bool is_advertiser, int *instance, enum ORB_PRIO priority)
{
	int ret = -1;

	char nodepath[ORB_MAXPATH];
	ret = uorb_node_mkpath(nodepath, meta, instance);
	if (ret != 0) {
		return ret;
	}

	KMDEBUG("ad enter\r\n");
	ret = -1;

	/* try for topic groups */
	const unsigned max_group_tries = (instance != NULL) ? ORB_MULTI_MAX_INSTANCES : 1;
	unsigned group_tries = 0;

	if (instance) {
		/* for an advertiser, this will be 0, but a for subscriber that requests a certain instance,
		 * we do not want to start with 0, but with the instance the subscriber actually requests.
		 */
		group_tries = *instance;

		if (group_tries >= max_group_tries) {
			KMDEBUG("advertise exit 4\r\n");
			return -ENOMEM;
		}
	}

    do {} while (sem_wait(&master->_lock) != 0);

	do {
		/* if path is modifyable change try index */
		if (instance != NULL) {
			/* replace the number at the end of the string */
			nodepath[strlen(nodepath) - 1] = '0' + group_tries;
			*instance = group_tries;
		}

		/* driver wants a permanent copy of the path, so make one here */
		const char *devpath = strdup(nodepath);

		if (devpath == NULL) {
            sem_post(&master->_lock);
			KMDEBUG("advertise exit 3 %s\r\n", nodepath);
			return -ENOMEM;
		}

		/* construct the new node, passing the ownership of path to it */
        struct __uorb_device_node *node = pvPortMalloc(sizeof(struct __uorb_device_node));
        uorb_device_node_init(node, meta, group_tries, devpath, priority, 1);

		/* if we didn't get a device, that's bad, free the path too */
		if (node == NULL) {
			free((void *)devpath);
			// vPortFree();
            sem_post(&master->_lock);
			KMDEBUG("advertise exit 2 %u\r\n", xPortGetFreeHeapSize());
			return -ENOMEM;
		}

		/* initialise the node - this may fail if e.g. a node with this name already exists */
		KMDEBUG("master register %d \r\n", *instance);
		ret = uorbnode_register(&node->nd); //register driver

		/* if init failed, discard the node and its name */
		if (ret != 0) {
			KMDEBUG("master unregister %d \r\n", *instance);
            uorb_device_node_deinit(node);
            vPortFree(node);

			KMDEBUG("master unregister2 %d \r\n", *instance);
			if (ret == -EEXIST) {
				/* if the node exists already, get the existing one and check if it's advertised. */
				struct __uorb_device_node *existing_node = uorb_device_master_get_node_locked(master, meta, group_tries);

				/*
				 * We can claim an existing node in these cases:
				 * - The node is not advertised (yet). It means there is already one or more subscribers or it was
				 *   unadvertised.
				 * - We are a single-instance advertiser requesting the first instance.
				 *   (Usually we don't end up here, but we might in case of a race condition between 2
				 *   advertisers).
				 * - We are a subscriber requesting a certain instance.
				 *   (Also we usually don't end up in that case, but we might in case of a race condtion
				 *   between an advertiser and subscriber).
				 */
				bool is_single_instance_advertiser = is_advertiser && !instance;

				if (existing_node != NULL &&
                    (!uorb_device_node_is_advertised(existing_node) || is_single_instance_advertiser || !is_advertiser)) {
					if (is_advertiser) {
                        uorb_device_node_set_priority(existing_node, (enum ORB_PRIO)priority);

						/* Set as advertised to avoid race conditions (otherwise 2 multi-instance advertisers
						 * could get the same instance).
						 */
                        uorb_device_node_mark_as_advertised(existing_node);
					}

					ret = 0;

				} else {
					/* otherwise: already advertised, keep looking */
				}
			}

			KMDEBUG("master unregister3 %d \r\n", *instance);
		} else {
			if (is_advertiser) {
                uorb_device_node_mark_as_advertised(node);
			}

			// add to the node map.
            intrusive_sorted_list_add(&master->_node_list, &node->sorted_list_node);
            node_bits_set(&master->_node_exists[uorb_device_node_get_instance(node)], uorb_device_node_id(node), true);
		}

		group_tries++;

	} while (ret != 0 && (group_tries < max_group_tries));

	if (ret != 0 && group_tries >= max_group_tries) {
		KMDEBUG("advertise exit 1\r\n");
		ret = -ENOMEM;
	}

    sem_post(&master->_lock);

	KMDEBUG("ad end %d\r\n", ret);
	return ret;
}

struct __uorb_device_node *uorb_device_master_get_node(struct __uorb_device_master *master, const char *nodepath)
{
    struct intrusive_node *iterator = NULL;
    struct __uorb_device_node *node = NULL;

	uorb_device_master_lock(master);

    for (iterator = master->_node_list.head; iterator != NULL; iterator = iterator->next) {
        node = container_of(iterator, struct __uorb_device_node, sorted_list_node);

        if (strcmp(uorb_device_node_get_name(node), nodepath) == 0) {
            uorb_device_master_unlock(master);
            return node;
        }
    }

	uorb_device_master_unlock(master);

	return NULL;
}

bool uorb_device_master_node_exists(struct __uorb_device_master *master, enum ORB_ID id, const uint8_t instance)
{
    if ((id == ORB_ID_INVALID) || (instance > ORB_MULTI_MAX_INSTANCES - 1)) {
        return false;
    }

    return node_bits_test(&master->_node_exists[instance], (uint8_t)id);
}


struct __uorb_device_node *uorb_device_master_get_node2(struct __uorb_device_master *master, const struct orb_metadata *meta, const uint8_t instance)
{
    if (meta == NULL) {
        return NULL;
    }

    if (!uorb_device_master_node_exists(master, (enum ORB_ID)(meta->o_id), instance)) {
        return NULL;
    }

    uorb_device_master_lock(master);
    struct __uorb_device_node *node = uorb_device_master_get_node_locked(master, meta, instance);
    uorb_device_master_unlock(master);

    //We can safely return the node that can be used by any thread, because a DeviceNode never gets deleted.
    return node;
}

void uorb_device_master_print_statistics(struct __uorb_device_master *master)
{
    /* Add all nodes to a list while locked, and then print them in unlocked state, to avoid potential
        * dead-locks (where printing blocks) */
    uorb_device_master_lock(master);
    struct __device_node_static_data *first_node = NULL;
    struct __device_node_static_data *cur_node = NULL;
    size_t max_topic_name_length = 0;
    int num_topics = 0;
    int ret = uorb_device_master_add_new_nodes(master, &first_node, &num_topics, &max_topic_name_length, NULL, 0);
    uorb_device_master_unlock(master);

    if (ret != 0) {
        KMERROR("addNewDeviceNodes failed (%i) \n", ret);
        return;
    }

    KMINFO("%-*s INST #SUB #Q SIZE PRIO PATH\r\n", (int)max_topic_name_length - 2, "TOPIC NAME");

    cur_node = first_node;

    while (cur_node) {
        uorb_device_node_print_statistics(cur_node->node, max_topic_name_length);

        struct __device_node_static_data *prev = cur_node;
        cur_node = cur_node->next;
        vPortFree(prev);
    }
}

// #define CLEAR_LINE "\033[K"
#define CLEAR_LINE

void uorb_device_master_showtop(struct __uorb_device_master *master, char **topic_filter, int num_filters)
{
	bool print_active_only = true;
	bool only_once = false; // if true, run only once, then exit

	if (topic_filter && num_filters > 0) {
		bool show_all = false;

		for (int i = 0; i < num_filters; ++i) {
			if (!strcmp("-a", topic_filter[i])) {
				show_all = true;

			} else if (!strcmp("-1", topic_filter[i])) {
				only_once = true;
			}
		}

		print_active_only = only_once ? (num_filters == 1) : false; // print non-active if -a or some filter given

		if (show_all || print_active_only) {
			num_filters = 0;
		}
	}

	// KMINFO("\033[2J\n"); //clear screen

	uorb_device_master_lock(master);

	if (intrusive_sorted_list_empty(&master->_node_list)) {
		uorb_device_master_unlock(master);
		KMINFO("no active topics");
		return;
	}

	struct __device_node_static_data *first_node = NULL;
	struct __device_node_static_data *cur_node = NULL;
	size_t max_topic_name_length = 0;
	int num_topics = 0;
	int ret = uorb_device_master_add_new_nodes(master, &first_node, &num_topics, &max_topic_name_length, topic_filter, num_filters); 

    /* a DeviceNode is never deleted, so it's save to unlock here and still access the DeviceNodes */
    uorb_device_master_unlock(master);

	if (ret != 0) {
		KMERROR("addNewDeviceNodes failed (%i)", ret);
	}

	only_once = true;
	bool quit = false;
	hrt_abstime start_time = hrt_absolute_time();

	while (!quit) {
		if (!quit) {

			//update the stats
			hrt_abstime current_time = hrt_absolute_time();
			float dt = (current_time - start_time) / 1.e6f;
			cur_node = first_node;

			while (cur_node) {
				unsigned int num_msgs = uorb_device_node_published_message_count(cur_node->node);
				cur_node->pub_msg_delta = roundf((num_msgs - cur_node->last_pub_msg_count) / dt);
				cur_node->last_pub_msg_count = num_msgs;
				cur_node = cur_node->next;
			}

			start_time = current_time;

			if (!only_once) {
				KMINFO("\033[H"); // move cursor to top left corner
			}

			KMINFO(CLEAR_LINE "update: 1s, num topics: %i\n", num_topics);
			KMINFO(CLEAR_LINE "%-*s INST #SUB RATE #Q SIZE\n", (int)max_topic_name_length - 2, "TOPIC NAME");
			cur_node = first_node;

			while (cur_node) {

				if (!print_active_only || (cur_node->pub_msg_delta > 0 && uorb_device_node_subscriber_count(cur_node->node) > 0)) {
                    struct orb_metadata *meta = uorb_device_node_get_meta(cur_node->node);
					KMINFO(CLEAR_LINE "%-*s %2i %4i %4i %2i %4i \n", (int)max_topic_name_length,
                            meta->o_name, (int)uorb_device_node_get_instance(cur_node->node),
                            (int)uorb_device_node_subscriber_count(cur_node->node), cur_node->pub_msg_delta,
                            uorb_device_node_get_queue_size(cur_node->node), meta->o_size);
				}

				cur_node = cur_node->next;
			}

			if (!only_once) {
				KMINFO("\033[0J"); // clear the rest of the screen
			}

			uorb_device_master_lock(master);
			ret = uorb_device_master_add_new_nodes(master, &first_node, &num_topics, &max_topic_name_length, topic_filter, num_filters);
			uorb_device_master_unlock(master);

			if (ret != 0) {
				KMERROR("addNewDeviceNodes failed (%i)", ret);
			}

		}

		if (only_once) {
			quit = true;
		}
	}

	//cleanup
	cur_node = first_node;

	while (cur_node) {
		struct __device_node_static_data *next_node = cur_node->next;
        vPortFree(cur_node);
		cur_node = next_node;
	}
}
