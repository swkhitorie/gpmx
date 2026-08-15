#ifndef UORB_MANAGER_H_
#define UORB_MANAGER_H_

#include <gpmx/config.h>

#include "uorb_common.h"
#include "uorb_device_master.h"

#if defined(CONFIG_ORB_USE_PUBLISHER_RULES)
#define ORB_USE_PUBLISHER_RULES
#endif

struct __uorb_manager {
    struct __uorb_device_master *_device_master;
};

#ifdef ORB_USE_PUBLISHER_RULES
struct publisher_rule {
	const char **topics; //null-terminated list of topic names
	const char *module_name; //only this module is allowed to publish one of the topics
	bool ignore_other_topics;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

bool uorb_manager_initialize();
bool uorb_manager_terminate();
struct __uorb_manager *uorb_manager_instance();
struct __uorb_device_master *uorb_manager_get_device_master(struct __uorb_manager *manager);

orb_advert_t orb_manager_advertise(struct __uorb_manager *manager, const struct orb_metadata *meta, const void *data, unsigned int queue_size);
orb_advert_t orb_manager_advertise_multi(struct __uorb_manager *manager, const struct orb_metadata *meta, const void *data, int *instance, 
                    enum ORB_PRIO priority, unsigned int queue_size);
int orb_manager_unadvertise(struct __uorb_manager *manager, orb_advert_t handle);
int orb_manager_publish(struct __uorb_manager *manager, const struct orb_metadata *meta, orb_advert_t handle, const void *data);
int orb_manager_subscribe(struct __uorb_manager *manager, const struct orb_metadata *meta);
int orb_manager_subscribe_multi(struct __uorb_manager *manager, const struct orb_metadata *meta, unsigned instance);
int orb_manager_unsubscribe(struct __uorb_manager *manager, int fd);
int orb_manager_copy(struct __uorb_manager *manager, const struct orb_metadata *meta, int handle, void *buffer);
int orb_manager_check(struct __uorb_manager *manager, int handle, bool *updated);
int orb_manager_exists(struct __uorb_manager *manager, const struct orb_metadata *meta, int instance);
int orb_manager_priority(struct __uorb_manager *manager, int handle, enum ORB_PRIO *priority);
int orb_manager_set_interval(struct __uorb_manager *manager, int handle, unsigned interval);
int	orb_manager_get_interval(struct __uorb_manager *manager, int handle, unsigned *interval);

#ifdef ORB_USE_PUBLISHER_RULES
void publisher_rule_print(const struct publisher_rule *rule);
void publisher_rule_printl();
int read_publisher_rules_from_file(const char *file_content, ssize_t file_length, struct publisher_rule *rule);
int publisher_rule_config(const char *file_content, ssize_t file_length);
#endif

#ifdef __cplusplus
}
#endif

#endif
