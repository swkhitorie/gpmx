#ifndef SUBSCRIPTION_CALLBACK_H_
#define SUBSCRIPTION_CALLBACK_H_

#include "uorb_common.h"
#include "uorb_device_node.h"
#include "uorb_manager.h"

#include "subscription_interval.h"
#include "container/intrusive_node.h"
#include "workitem.h"

typedef void (*sub_callback)(struct __subscription_callback *);

struct __subscription_callback {
    struct intrusive_node list_node;

    struct __subscription_interval _subscription_itv;

    bool _registered;
    sub_callback call;
};

struct __subscription_callback_workitem {
    struct __subscription_callback _sub_callback;

    struct __workitem *_work_item;
    uint8_t _required_updates;
};

#ifdef __cplusplus
extern "C" {
#endif

void subscription_callback_init(struct __subscription_callback *sub_cal, const struct orb_metadata *meta, uint32_t interval_us, uint8_t instance);
void subscription_callback_deinit(struct __subscription_callback *sub_cal);
bool subscription_callback_register(struct __subscription_callback *sub_cal);
void subscription_callback_unregister(struct __subscription_callback *sub_cal);

void subscription_callback_workitem_init(struct __subscription_callback_workitem *sub_cal_witem, struct __workitem *work_item, 
    const struct orb_metadata *meta, uint8_t instance);
void subscription_callback_workitem_set_required_updates(struct __subscription_callback_workitem *sub_cal_witem, uint8_t required_updates);


#ifdef __cplusplus
}
#endif

#endif
