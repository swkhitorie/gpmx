#include "ScheduledWorkItemDemo.hpp"

#define MODULE_NAME "workItemEx"
SWorkItemExample::SWorkItemExample() :
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::test1)
{
}

SWorkItemExample::~SWorkItemExample()
{
	perf_free(_loop_perf);
	perf_free(_loop_interval_perf);
}

bool SWorkItemExample::init()
{
    PX4_INFO("init() called %.5f \r\n", hrt_absolute_time()/1e6f);

	return true;
}

void SWorkItemExample::Run()
{
	if (should_exit()) {
		ScheduleClear();
		exit_and_cleanup();
		return;
	}

	perf_begin(_loop_perf);
	perf_count(_loop_interval_perf);

    PX4_INFO("Run() called %.5f \r\n", hrt_absolute_time()/1e6f);
    // ScheduleDelayed(100_ms);

	perf_end(_loop_perf);
}

int SWorkItemExample::task_spawn(int argc, char *argv[])
{
	SWorkItemExample *instance = new SWorkItemExample();

	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;

		if (instance->init()) {
            PX4_INFO("init ok");
            instance->ScheduleOnInterval(100*1000);
			return PX4_OK;
		}

	} else {
		PX4_ERR("alloc failed");
	}

	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

int SWorkItemExample::print_status()
{
	perf_print_counter(_loop_perf);
	perf_print_counter(_loop_interval_perf);
	return 0;
}

int SWorkItemExample::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

int SWorkItemExample::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
Example of a simple module running out of a work queue.

)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("SWorkItemExample", "template");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();

	return 0;
}

extern "C" __EXPORT int swork_item_example_main(int argc, char *argv[])
{
	return SWorkItemExample::main(argc, argv);
}