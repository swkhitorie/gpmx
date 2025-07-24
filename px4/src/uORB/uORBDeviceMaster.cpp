/****************************************************************************
 *
 *   Copyright (c) 2012-2016 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "uORBDeviceMaster.hpp"
#include "uORBDeviceNode.hpp"
#include "uORBManager.hpp"
#include "uORBUtils.hpp"

#ifdef ORB_COMMUNICATOR
#include "uORBCommunicator.hpp"
#endif /* ORB_COMMUNICATOR */

#include <px4_platform_common/sem.hpp>
// #include <systemlib/px4_macros.h>

#include <math.h>

#ifndef __PX4_QURT // QuRT has no poll()
// #include <poll.h>
#endif // PX4_QURT

uORB::DeviceMaster::DeviceMaster()
{
	px4_sem_init(&_lock, 0, 1);
}

uORB::DeviceMaster::~DeviceMaster()
{
	px4_sem_destroy(&_lock);
}

int uORB::DeviceMaster::advertise(const struct orb_metadata *meta, bool is_advertiser, int *instance, ORB_PRIO priority)
{
	int ret = PX4_ERROR;

	char nodepath[orb_maxpath];

	/* construct a path to the node - this also checks the node name */
	ret = uORB::Utils::node_mkpath(nodepath, meta, instance);

	if (ret != PX4_OK) {
		return ret;
	}

	ret = PX4_ERROR;

	/* try for topic groups */
	const unsigned max_group_tries = (instance != nullptr) ? ORB_MULTI_MAX_INSTANCES : 1;
	unsigned group_tries = 0;

	if (instance) {
		/* for an advertiser, this will be 0, but a for subscriber that requests a certain instance,
		 * we do not want to start with 0, but with the instance the subscriber actually requests.
		 */
		group_tries = *instance;

		if (group_tries >= max_group_tries) {
			return -ENOMEM;
		}
	}

	SmartLock smart_lock(_lock);

	do {
		/* if path is modifyable change try index */
		if (instance != nullptr) {
			/* replace the number at the end of the string */
			nodepath[strlen(nodepath) - 1] = '0' + group_tries;
			*instance = group_tries;
		}

		/* driver wants a permanent copy of the path, so make one here */
		const char *devpath = strdup(nodepath);

		if (devpath == nullptr) {
			return -ENOMEM;
		}

		/* construct the new node, passing the ownership of path to it */
		uORB::DeviceNode *node = new uORB::DeviceNode(meta, group_tries, devpath, priority);

		/* if we didn't get a device, that's bad, free the path too */
		if (node == nullptr) {
			px4_free((void *)devpath);
			return -ENOMEM;
		}

		/* initialise the node - this may fail if e.g. a node with this name already exists */
		ret = node->init();

		/* if init failed, discard the node and its name */
		if (ret != PX4_OK) {
			delete node;

			if (ret == -EEXIST) {
				/* if the node exists already, get the existing one and check if it's advertised. */
				uORB::DeviceNode *existing_node = getDeviceNodeLocked(meta, group_tries);

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

				if (existing_node != nullptr &&
				    (!existing_node->is_advertised() || is_single_instance_advertiser || !is_advertiser)) {
					if (is_advertiser) {
						existing_node->set_priority((ORB_PRIO)priority);
						/* Set as advertised to avoid race conditions (otherwise 2 multi-instance advertisers
						 * could get the same instance).
						 */
						existing_node->mark_as_advertised();
					}

					ret = PX4_OK;

				} else {
					/* otherwise: already advertised, keep looking */
				}
			}

		} else {
			if (is_advertiser) {
				node->mark_as_advertised();
			}

			// add to the node map.
			_node_list.add(node);
			_node_exists[node->get_instance()].set((uint8_t)node->id(), true);
		}

		group_tries++;

	} while (ret != PX4_OK && (group_tries < max_group_tries));

	if (ret != PX4_OK && group_tries >= max_group_tries) {
		ret = -ENOMEM;
	}

	return ret;
}

void uORB::DeviceMaster::printStatistics()
{
	/* Add all nodes to a list while locked, and then print them in unlocked state, to avoid potential
	 * dead-locks (where printing blocks) */
	lock();
	DeviceNodeStatisticsData *first_node = nullptr;
	DeviceNodeStatisticsData *cur_node = nullptr;
	size_t max_topic_name_length = 0;
	int num_topics = 0;
	int ret = addNewDeviceNodes(&first_node, num_topics, max_topic_name_length, nullptr, 0);
	unlock();

	if (ret != 0) {
		PX4_ERR("addNewDeviceNodes failed (%i)", ret);
		return;
	}

	PX4_INFO_RAW("%-*s INST #SUB #Q SIZE PRIO PATH\n", (int)max_topic_name_length - 2, "TOPIC NAME");

	cur_node = first_node;

	while (cur_node) {
		cur_node->node->print_statistics(max_topic_name_length);

		DeviceNodeStatisticsData *prev = cur_node;
		cur_node = cur_node->next;
		delete prev;
	}
}

int uORB::DeviceMaster::addNewDeviceNodes(DeviceNodeStatisticsData **first_node, int &num_topics,
		size_t &max_topic_name_length, char **topic_filter, int num_filters)
{
	DeviceNodeStatisticsData *cur_node = nullptr;
	num_topics = 0;
	DeviceNodeStatisticsData *last_node = *first_node;

	if (last_node) {
		while (last_node->next) {
			last_node = last_node->next;
		}
	}

	for (const auto &node : _node_list) {

		++num_topics;

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
				if (strstr(node->get_meta()->o_name, topic_filter[i])) {
					matched = true;
				}
			}

			if (!matched) {
				continue;
			}
		}

		if (last_node) {
			last_node->next = new DeviceNodeStatisticsData();
			last_node = last_node->next;

		} else {
			*first_node = last_node = new DeviceNodeStatisticsData();
		}

		if (!last_node) {
			return -ENOMEM;
		}

		last_node->node = node;

		size_t name_length = strlen(last_node->node->get_meta()->o_name);

		if (name_length > max_topic_name_length) {
			max_topic_name_length = name_length;
		}

		last_node->last_pub_msg_count = last_node->node->published_message_count();
	}

	return 0;
}

#define CLEAR_LINE "\033[K"

void uORB::DeviceMaster::showTop(char **topic_filter, int num_filters)
{

}

#undef CLEAR_LINE

uORB::DeviceNode *uORB::DeviceMaster::getDeviceNode(const char *nodepath)
{
	lock();

	for (uORB::DeviceNode *node : _node_list) {
		if (strcmp(node->get_devname(), nodepath) == 0) {
			unlock();
			return node;
		}
	}

	unlock();

	return nullptr;
}

bool uORB::DeviceMaster::deviceNodeExists(ORB_ID id, const uint8_t instance)
{
	if ((id == ORB_ID::INVALID) || (instance > ORB_MULTI_MAX_INSTANCES - 1)) {
		return false;
	}

	return _node_exists[instance][(uint8_t)id];
}

uORB::DeviceNode *uORB::DeviceMaster::getDeviceNode(const struct orb_metadata *meta, const uint8_t instance)
{
	if (meta == nullptr) {
		return nullptr;
	}

	if (!deviceNodeExists(static_cast<ORB_ID>(meta->o_id), instance)) {
		return nullptr;
	}

	lock();
	uORB::DeviceNode *node = getDeviceNodeLocked(meta, instance);
	unlock();

	//We can safely return the node that can be used by any thread, because
	//a DeviceNode never gets deleted.
	return node;
}

uORB::DeviceNode *uORB::DeviceMaster::getDeviceNodeLocked(const struct orb_metadata *meta, const uint8_t instance)
{
	for (uORB::DeviceNode *node : _node_list) {
		if ((strcmp(node->get_name(), meta->o_name) == 0) && (node->get_instance() == instance)) {
			return node;
		}
	}

	return nullptr;
}
