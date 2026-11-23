#pragma once

#include "WorkItem.hpp"

namespace px4
{

class ScheduledWorkItem : public WorkItem
{
public:

	bool Scheduled() { return !hrt_called(&_call); }

	/**
	 * Schedule next run with a delay in microseconds.
	 */
	void ScheduleDelayed(uint32_t delay_us);

	/**
	 * Schedule repeating run with optional delay.
	 */
	void ScheduleOnInterval(uint32_t interval_us, uint32_t delay_us = 0);

	/**
	 * Schedule next run at a specific time.
	 */
	void ScheduleAt(hrt_abstime time_us);

	/**
	 * Clear any scheduled work.
	 */
	void ScheduleClear();

protected:

	ScheduledWorkItem(const char *name, const wq_config_t &config) : WorkItem(name, config) {}
	virtual ~ScheduledWorkItem() override;

	virtual void print_run_status() override;

private:

	virtual void Run() override = 0;

	static void ScheduleTrampoline(void *arg);

	struct hrt_call _call {};
};

} // namespace px4
