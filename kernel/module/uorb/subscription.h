#ifndef SUBSCRIPTION_H_
#define SUBSCRIPTION_H_

#include <uorb/topics/uorb_topics.h>

#include "uorb_common.h"
#include "uorb_device_node.h"
#include "uorb_manager.h"

struct __subscription_callback;

struct __subscription {
	struct __uorb_device_node *_node;

	uint32_t _last_generation; /**< last generation the subscriber has seen */

	enum ORB_ID _orb_id;
	uint8_t _instance;
};

#ifdef __cplusplus
extern "C" {
#endif

void subscription_init(struct __subscription *sub, enum ORB_ID id, uint8_t instance);
void subscription_init2(struct __subscription *sub, const struct orb_metadata *meta, uint8_t instance);
void subscription_deinit(struct __subscription *sub);

bool subscription_subscribe(struct __subscription *sub);
void subscription_unsubscribe(struct __subscription *sub);

bool subscription_valid(struct __subscription *sub);
bool subscription_advertised(struct __subscription *sub);

bool subscription_updated(struct __subscription *sub);
bool subscription_update(struct __subscription *sub, void *dst);
bool subscription_copy(struct __subscription *sub, void *dst);

uint8_t  subscription_get_instance(struct __subscription *sub);
uint32_t subscription_get_last_generation(struct __subscription *sub);
enum ORB_PRIO subscription_get_priority(struct __subscription *sub);
orb_id_t subscription_get_topic(struct __subscription *sub);

struct __uorb_device_node *subscription_get_node(struct __subscription *sub);

#ifdef __cplusplus
}
#endif

#endif
