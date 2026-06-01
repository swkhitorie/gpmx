#ifndef UORB_MANAGER_H_
#define UORB_MANAGER_H_

#include "uorb_common.h"
#include "uorb_device_master.h"

struct __uorb_manager {
    struct __uorb_device_master *_device_master;
};

#ifdef __cplusplus
extern "C" {
#endif

bool uorb_manager_initialize();
bool uorb_manager_terminate();
struct __uorb_manager *uorb_manager_instance();
struct __uorb_device_master *uorb_manager_get_device_master(struct __uorb_manager *manager);

orb_advert_t orb_manager_advertise(struct __uorb_manager *manager, struct orb_metadata *meta, const void *data, unsigned int queue_size);
orb_advert_t orb_manager_advertise_multi(struct __uorb_manager *manager, struct orb_metadata *meta, const void *data, int *instance, 
                    enum ORB_PRIO priority, unsigned int queue_size);
int orb_manager_unadvertise(struct __uorb_manager *manager, orb_advert_t handle);
int orb_manager_publish(struct __uorb_manager *manager, struct orb_metadata *meta, orb_advert_t handle, const void *data);
int orb_manager_subscribe(struct __uorb_manager *manager, struct orb_metadata *meta);
int orb_manager_subscribe_multi(struct __uorb_manager *manager, struct orb_metadata *meta, unsigned instance);
int orb_manager_unsubscribe(struct __uorb_manager *manager, int fd);
int orb_manager_copy(struct __uorb_manager *manager, struct orb_metadata *meta, int handle, void *buffer);
int orb_manager_check(struct __uorb_manager *manager, int handle, bool *updated);
int orb_manager_exists(struct __uorb_manager *manager, struct orb_metadata *meta, int instance);
int orb_manager_priority(struct __uorb_manager *manager, int handle, enum ORB_PRIO *priority);
int orb_manager_set_interval(struct __uorb_manager *manager, int handle, unsigned interval);
int	orb_manager_get_interval(struct __uorb_manager *manager, int handle, unsigned *interval);

#ifdef __cplusplus
}
#endif

#endif
