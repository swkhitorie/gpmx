#ifndef UORB_DEVICE_NODE_H_
#define UORB_DEVICE_NODE_H_

#include "uorb_common.h"
#include "uorb_device_master.h"
#include "container/atomic.h"
#include "container/intrusive_node.h"
#include "uorb_gnode.h"

struct __subscription_callback;

struct __uorb_device_node {
    struct intrusive_node sorted_list_node;

    struct orb_metadata *_meta; /**< object metadata information */

    uint8_t    *_data;   /**< allocated object buffer */
    atomic_uint32_t _generation;  /**< object generation count */

    struct intrusive_list_head _callbacks;
    // List<uORB::SubscriptionCallback *>	_callbacks;

    enum ORB_PRIO _priority;  /**< priority of the topic */
    uint8_t _instance; /**< orb multi instance identifier */
    bool _advertised;  /**< has ever been advertised (not necessarily published data yet) */
    uint8_t _queue_size; /**< maximum number of elements in the queue */
    int8_t _subscriber_count;

    struct urbnode nd;
};

#ifdef __cplusplus
extern "C" {
#endif

void uorb_device_node_init(struct __uorb_device_node *unode, struct orb_metadata *meta, const uint8_t instance, 
            const char *path, enum ORB_PRIO priority, uint8_t queue_size);
void uorb_device_node_deinit(struct __uorb_device_node *unode);

int uorb_device_node_open(struct urbnode *node);
int uorb_device_node_close(struct urbnode *node);
ssize_t uorb_device_node_read(struct urbnode *node, char *buffer, size_t buflen);
ssize_t	uorb_device_node_write(struct urbnode *node, const char *buffer, size_t buflen);
int	uorb_device_node_ioctl(struct urbnode *node, int cmd, unsigned long arg);

ssize_t uorb_device_node_publish(const struct orb_metadata *meta, orb_advert_t handle, const void *data);
int uorb_device_node_unadvertise(orb_advert_t handle);

void uorb_device_node_add_internal_subscriber(struct __uorb_device_node *unode);
void uorb_device_node_remove_internal_subscriber(struct __uorb_device_node *unode);

bool uorb_device_node_is_advertised(struct __uorb_device_node *unode);
void uorb_device_node_mark_as_advertised(struct __uorb_device_node *unode);
uint8_t uorb_device_node_get_queue_size(struct __uorb_device_node *unode);
int8_t uorb_device_node_subscriber_count(struct __uorb_device_node *unode);
uint32_t uorb_device_node_published_message_count(struct __uorb_device_node *unode);
enum ORB_ID uorb_device_node_id(struct __uorb_device_node *unode);
const char *uorb_device_node_get_name(struct __uorb_device_node *unode);
uint8_t uorb_device_node_get_instance(struct __uorb_device_node *unode);
enum ORB_PRIO uorb_device_node_get_priority(struct __uorb_device_node *unode);
void uorb_device_node_set_priority(struct __uorb_device_node *unode, enum ORB_PRIO priority);
struct orb_metadata *uorb_device_node_get_meta(struct __uorb_device_node *unode);

bool uorb_device_node_print_statistics(struct __uorb_device_node *unode, int max_topic_length);

/**
 * Try to change the size of the queue. This can only be done as long as nobody published yet.
 * This is the case, for example when orb_subscribe was called before an orb_advertise.
 * The queue size can only be increased.
 */
int uorb_device_node_update_queue_size(struct __uorb_device_node *unode, unsigned int queue_size);

/**
 * Copies data and the corresponding generation
 * from a node to the buffer provided.
 */
bool uorb_device_node_copy(struct __uorb_device_node *unode, void *dst, uint32_t *generation);

// add item to list of work items to schedule on node update
bool uorb_device_node_register_callback(struct __uorb_device_node *unode, struct __subscription_callback *callback_sub);

// remove item from list of work items
void uorb_device_node_unregister_callback(struct __uorb_device_node *unode, struct __subscription_callback *callback_sub);

bool uorb_device_cmp_method(struct intrusive_node *a, struct intrusive_node *b);

#ifdef __cplusplus
}
#endif

#endif
