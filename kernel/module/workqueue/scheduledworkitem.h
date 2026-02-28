#ifndef SCHEDULED_WORKITEM_H_
#define SCHEDULED_WORKITEM_H_

#include "workitem.h"
#include "drv_hrt.h"

struct __scheduledworkitem {
    struct __workitem item;

    struct hrt_call _call;
};


#ifdef __cplusplus
extern "C" {
#endif

void schedule_workitem_deinit(struct __scheduledworkitem *sitem);

void schedule_workitem_delayed(struct __scheduledworkitem *sitem, uint32_t delay_us);
void schedule_workitem_oninterval(struct __scheduledworkitem *sitem, uint32_t interval_us, uint32_t delay_us);
void schedule_workitem_clear(struct __scheduledworkitem *sitem);

void schedule_workitem_print_status(struct __scheduledworkitem *sitem);

#ifdef __cplusplus
}
#endif

#endif
