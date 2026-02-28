#include "uorb_common.h"
#include "uorb_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

orb_advert_t orb_advertise(struct orb_metadata *meta, const void *data)
{
	return orb_manager_advertise(uorb_manager_instance(), meta, data, 1);
}

orb_advert_t orb_advertise_queue(struct orb_metadata *meta, const void *data, unsigned int queue_size)
{
	return orb_manager_advertise(uorb_manager_instance(), meta, data, queue_size);
}

orb_advert_t orb_advertise_multi(struct orb_metadata *meta, const void *data, int *instance, enum ORB_PRIO priority)
{
	return orb_manager_advertise_multi(uorb_manager_instance(), meta, data, instance, priority, 1);
}

orb_advert_t orb_advertise_multi_queue(struct orb_metadata *meta, const void *data, int *instance,
                            enum ORB_PRIO priority, unsigned int queue_size)
{
	return orb_manager_advertise_multi(uorb_manager_instance(), meta, data, instance, priority, queue_size);
}

int orb_unadvertise(orb_advert_t handle)
{
	return orb_manager_unadvertise(uorb_manager_instance(), handle);
}

int orb_publish(struct orb_metadata *meta, orb_advert_t handle, const void *data)
{
	return orb_manager_publish(uorb_manager_instance(), meta, handle, data);
}

int orb_subscribe(struct orb_metadata *meta)
{
	return orb_manager_subscribe(uorb_manager_instance(), meta);
}

int orb_subscribe_multi(struct orb_metadata *meta, unsigned instance)
{
	return orb_manager_subscribe_multi(uorb_manager_instance(), meta, instance);
}

int orb_unsubscribe(int handle)
{
	return orb_manager_unsubscribe(uorb_manager_instance(), handle);
}

int orb_copy(struct orb_metadata *meta, int handle, void *buffer)
{
	return orb_manager_copy(uorb_manager_instance(), meta, handle, buffer);
}

int orb_check(int handle, bool *updated)
{
	return orb_manager_check(uorb_manager_instance(), handle, updated);
}

int orb_exists(struct orb_metadata *meta, int instance)
{
	return orb_manager_exists(uorb_manager_instance(), meta, instance);
}

int orb_group_count(struct orb_metadata *meta)
{
	unsigned instance = 0;

	while (orb_manager_exists(uorb_manager_instance(), meta, instance) == 0) {
		++instance;
	};

	return instance;
}

int orb_priority(int handle, enum ORB_PRIO *priority)
{
	return orb_manager_priority(uorb_manager_instance(), handle, priority);
}

int orb_set_interval(int handle, unsigned interval)
{
	return orb_manager_set_interval(uorb_manager_instance(), handle, interval);
}

int orb_get_interval(int handle, unsigned *interval)
{
	return orb_manager_get_interval(uorb_manager_instance(), handle, interval);
}

int uorb_node_mkpath(char *buf, struct orb_metadata *meta, int *instance)
{
    unsigned len;
    unsigned index = 0;

    if (instance != NULL) {
        index = *instance;
    }

    len = snprintf(buf, ORB_MAXPATH, "/%s/%s%d", "obj", meta->o_name, index);
    if (len >= ORB_MAXPATH) {
        return -ENAMETOOLONG;
    }

    return 0;
}

enum TESTENUM2 {
	TEAST1,
	TEAST2
};

int uorb_node_mkpath2(char *buf, const char *orbMsgName)
{
    unsigned len;
    unsigned index = 0;

    len = snprintf(buf, ORB_MAXPATH, "/%s/%s%d", "obj", orbMsgName, index);
    if (len >= ORB_MAXPATH) {
        return -91;
    }

    return 0;
}

