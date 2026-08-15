#include <gpmx/config.h>

#include "uorb_common.h"
#include "uorb_manager.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

orb_advert_t orb_advertise(const struct orb_metadata *meta, const void *data)
{
	return orb_manager_advertise(uorb_manager_instance(), meta, data, 1);
}

orb_advert_t orb_advertise_queue(const struct orb_metadata *meta, const void *data, unsigned int queue_size)
{
	return orb_manager_advertise(uorb_manager_instance(), meta, data, queue_size);
}

orb_advert_t orb_advertise_multi(const struct orb_metadata *meta, const void *data, int *instance, enum ORB_PRIO priority)
{
	return orb_manager_advertise_multi(uorb_manager_instance(), meta, data, instance, priority, 1);
}

orb_advert_t orb_advertise_multi_queue(const struct orb_metadata *meta, const void *data, int *instance,
                            enum ORB_PRIO priority, unsigned int queue_size)
{
	return orb_manager_advertise_multi(uorb_manager_instance(), meta, data, instance, priority, queue_size);
}

int orb_unadvertise(orb_advert_t handle)
{
	return orb_manager_unadvertise(uorb_manager_instance(), handle);
}

int orb_publish(const struct orb_metadata *meta, orb_advert_t handle, const void *data)
{
	return orb_manager_publish(uorb_manager_instance(), meta, handle, data);
}

int orb_subscribe(const struct orb_metadata *meta)
{
	return orb_manager_subscribe(uorb_manager_instance(), meta);
}

int orb_subscribe_multi(const struct orb_metadata *meta, unsigned instance)
{
	return orb_manager_subscribe_multi(uorb_manager_instance(), meta, instance);
}

int orb_unsubscribe(int handle)
{
	return orb_manager_unsubscribe(uorb_manager_instance(), handle);
}

int orb_copy(const struct orb_metadata *meta, int handle, void *buffer)
{
	return orb_manager_copy(uorb_manager_instance(), meta, handle, buffer);
}

int orb_check(int handle, bool *updated)
{
	return orb_manager_check(uorb_manager_instance(), handle, updated);
}

int orb_exists(const struct orb_metadata *meta, int instance)
{
	return orb_manager_exists(uorb_manager_instance(), meta, instance);
}

int orb_group_count(const struct orb_metadata *meta)
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

int uorb_node_mkpath(char *buf, const struct orb_metadata *meta, int *instance)
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

const char *orb_get_c_type(unsigned char short_type)
{
	// this matches with the uorb o_fields generator
	switch (short_type) {
	case 0x82: return "int8_t";

	case 0x83: return "int16_t";

	case 0x84: return "int32_t";

	case 0x85: return "int64_t";

	case 0x86: return "uint8_t";

	case 0x87: return "uint16_t";

	case 0x88: return "uint32_t";

	case 0x89: return "uint64_t";

	case 0x8a: return "float";

	case 0x8b: return "double";

	case 0x8c: return "bool";

	case 0x8d: return "char";
	}

	return NULL;
}

void orb_print_message_internal(const const struct orb_metadata *meta, const void *data, bool print_topic_name)
{

}

#if defined(CONFIG_GMSH)
#include "gmsh.h"
int uorb_main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "status")) {
        uorb_device_master_print_statistics(uorb_manager_get_device_master(uorb_manager_instance()));
    }

    if (argc == 2 && !strcmp(argv[1], "top")) {
        uorb_device_master_showtop(uorb_manager_get_device_master(uorb_manager_instance()), NULL, 0);
    }

	gsh_kprintf("Usage: uorb <status/top>\r\n");
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(uorb_main, uorb, check uorb status.);
#endif
