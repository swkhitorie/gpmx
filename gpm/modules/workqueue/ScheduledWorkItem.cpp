#include "ScheduledWorkItem.hpp"
#include <inttypes.h>

namespace px4
{

ScheduledWorkItem::~ScheduledWorkItem()
{
	if (_call.arg != nullptr) {
		ScheduleClear();
	}
}

void ScheduledWorkItem::ScheduleTrampoline(void *arg)
{
	static_cast<ScheduledWorkItem *>(arg)->ScheduleNow();
}

void ScheduledWorkItem::ScheduleDelayed(uint32_t delay_us)
{
	hrt_call_after(&_call, delay_us, (hrt_callout)&ScheduledWorkItem::ScheduleTrampoline, this);
}

void ScheduledWorkItem::ScheduleOnInterval(uint32_t interval_us, uint32_t delay_us)
{
	hrt_call_every(&_call, delay_us, interval_us, (hrt_callout)&ScheduledWorkItem::ScheduleTrampoline, this);
}

void ScheduledWorkItem::ScheduleAt(hrt_abstime time_us)
{
	hrt_call_at(&_call, time_us, (hrt_callout)&ScheduledWorkItem::ScheduleTrampoline, this);
}

void ScheduledWorkItem::ScheduleClear()
{
	hrt_cancel(&_call);

	WorkItem::ScheduleClear();
}

void ScheduledWorkItem::print_run_status()
{
	if (_call.period > 0) {
		KMRAW("%-29s %8.1f Hz %12.0f us (%" PRId64 " us)\n", ItemName(),
			    (double)average_rate(), (double)average_interval(), (int64_t)_call.period);

	} else {
		WorkItem::print_run_status();
	}
}

} // namespace px4
