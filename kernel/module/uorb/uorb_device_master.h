#ifndef UORB_DEVICE_MASTER_H_
#define UORB_DEVICE_MASTER_H_

#include "uorb_common.h"
#include "uorb_device_node.h"

#include <uorb/topics/uorb_topics.h>

#include "container/intrusive_node.h"
#include "container/atomic_bitset.h"

#include <string.h>
#include <stdlib.h>

#include <semaphore.h>

DEFINE_ATOMIC_BITSET(node_bits, ORB_TOPICS_COUNT)

struct __uorb_device_master {
    struct intrusive_sorted_list_head _node_list;

	node_bits _node_exists[ORB_MULTI_MAX_INSTANCES];

	sem_t _lock; /**< lock to protect access to all class members (also for derived classes) */
};

struct __device_node_static_data {
    struct __uorb_device_node *node;
    unsigned int last_pub_msg_count;
    unsigned int pub_msg_delta;
    struct __device_node_static_data *next;
};

#ifdef __cplusplus
extern "C" {
#endif

void uorb_device_master_init(struct __uorb_device_master *master);
void uorb_device_master_deinit(struct __uorb_device_master *master);

int uorb_device_master_advertise(struct __uorb_device_master *master, struct orb_metadata *meta, bool is_advertiser, int *instance, enum ORB_PRIO priority);

bool uorb_device_master_node_exists(struct __uorb_device_master *master, enum ORB_ID id, const uint8_t instance);

struct __uorb_device_node *uorb_device_master_get_node(struct __uorb_device_master *master, const char *node_name);
struct __uorb_device_node *uorb_device_master_get_node2(struct __uorb_device_master *master, const struct orb_metadata *meta, const uint8_t instance);

void uorb_device_master_print_statistics(struct __uorb_device_master *master);

void uorb_device_master_showtop(struct __uorb_device_master *master, char **topic_filter, int num_filters);



#ifdef __cplusplus
}
#endif

#endif
