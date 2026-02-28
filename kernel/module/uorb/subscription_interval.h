#ifndef SUBSCRIPTION_INTERVAL_H_
#define SUBSCRIPTION_INTERVAL_H_

#include "uorb_common.h"
#include "uorb_manager.h"

#include "subscription.h"

struct __subscription_interval {
    struct __subscription _sub;
    uint64_t _last_update;	// last update in microseconds
    uint32_t _interval_us;	// maximum update interval in microseconds
};

#ifdef __cplusplus
extern "C" {
#endif

void subscription_interval_init(struct __subscription_interval *subitv, enum ORB_ID id, uint32_t interval_us, uint8_t instance);
void subscription_interval_init2(struct __subscription_interval *subitv, const struct orb_metadata *meta, uint32_t interval_us, uint8_t instance);

bool subscription_interval_updated(struct __subscription_interval *subitv);
bool subscription_interval_update(struct __subscription_interval *subitv, void *dst);
bool subscription_interval_copy(struct __subscription_interval *subitv, void *dst);

uint32_t subscription_interval_get_us(struct __subscription_interval *subitv);
void subscription_interval_set_us(struct __subscription_interval *subitv, uint32_t interval);
void subscription_interval_set_ms(struct __subscription_interval *subitv, uint32_t interval);

#ifdef __cplusplus
}
#endif


#endif
