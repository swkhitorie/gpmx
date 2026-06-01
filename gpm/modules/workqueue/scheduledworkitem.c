#include "scheduledworkitem.h"
#include "kmodule_defines.h"

static void schedule_trampoline(void *arg)
{
	struct __scheduledworkitem *dev = (struct __scheduledworkitem *)(arg);
    workitem_schedule_now(&dev->item);
}

void schedule_workitem_deinit(struct __scheduledworkitem *sitem)
{
    workitem_schedule_clear(&sitem->item);
}

void schedule_workitem_delayed(struct __scheduledworkitem *sitem, uint32_t delay_us)
{
    hrt_call_after(&sitem->_call, delay_us, (hrt_callout)&schedule_trampoline, sitem);
}

void schedule_workitem_oninterval(struct __scheduledworkitem *sitem, uint32_t interval_us, uint32_t delay_us)
{
    hrt_call_every(&sitem->_call, delay_us, interval_us, (hrt_callout)&schedule_trampoline, sitem);
}

void schedule_workitem_clear(struct __scheduledworkitem *sitem)
{
    hrt_cancel(&sitem->_call);
    workitem_schedule_clear(&sitem->item);
}

void schedule_workitem_print_status(struct __scheduledworkitem *sitem)
{
    if (sitem->_call.period > 0) {
        KMINFO("%-26s %8.1f Hz %12.0f us (%" PRId64 " us)\n", sitem->item._name, (double)workitem_average_rate(&sitem->item),
                    (double)workitem_average_interval(&sitem->item), sitem->_call.period);
    } else {
        workitem_printstatus(&sitem->item);
    }
}
