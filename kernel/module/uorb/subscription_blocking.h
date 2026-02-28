#ifndef SUBSCRIPTION_BLOCKING_H_
#define SUBSCRIPTION_BLOCKING_H_

#include "uorb_common.h"
#include "uorb_device_node.h"
#include "uorb_manager.h"

#include "subscription_callback.h"
#include "container/intrusive_node.h"
#include <time.h>
#include <pthread.h>

struct __subscription_blocking {
    struct __subscription_callback _sub_callback;

	pthread_mutex_t _mutex;
	pthread_cond_t  _cv;
};

#ifdef __cplusplus
extern "C" {
#endif

void subscription_blocking_init(struct __subscription_blocking *sublk, const struct orb_metadata *meta, uint32_t interval_us, uint8_t instance);
void subscription_blocking_deinit(struct __subscription_blocking *sublk);

bool subscription_updated_blocking(struct __subscription_blocking *sublk, uint32_t timeout_us);
bool subscription_update_blocking2(struct __subscription_blocking *sublk, void *data, uint32_t timeout_us);

#ifdef __cplusplus
}
#endif



#endif
