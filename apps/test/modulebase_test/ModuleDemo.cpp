#include "ModuleDemo.hpp"

ModuleDemo::ModuleDemo()
{
	PX4_INFO("[ModuleDemo] constructor called %.5f\r\n", hrt_absolute_time()/1e6f);
}

ModuleDemo::~ModuleDemo()
{
	perf_free(_cycle_perf);
}

int ModuleDemo::print_status()
{
	PX4_INFO("Running");
	// TODO: print additional runtime information about the state of the module

	return 0;
}

int ModuleDemo::custom_command(int argc, char *argv[])
{
	/*
	if (!is_running()) {
		print_usage("not running");
		return 1;
	}

	// additional custom commands can be handled like this:
	if (!strcmp(argv[0], "do-something")) {
		get_instance()->do_something();
		return 0;
	}
	 */

	return print_usage("unknown command");
}

int ModuleDemo::task_spawn(int argc, char *argv[])
{
    _task_id = px4_task_spawn_cmd("module", 
                        SCHED_DEFAULT, 
						SCHED_PRIORITY_DEFAULT, 
                        1024,
						(px4_main_t)&run_trampoline,
						(char *const *)argv);

	if (_task_id < 0) {
		PX4_INFO("err task spawn\r\n");
		_task_id = -1;
		return -errno;
	}

	return 0;
}

ModuleDemo *ModuleDemo::instantiate(int argc, char *argv[])
{
	ModuleDemo *instance = new ModuleDemo();

	if (instance == nullptr) {
		PX4_ERR("alloc failed");
	}

	return instance;
}

void ModuleDemo::run()
{
	while (!should_exit()) {
		perf_begin(_cycle_perf);
		
		printf("[ModuleDemo] Run() called %.5f\r\n", hrt_absolute_time()/1e6f);

		perf_end(_cycle_perf);
		sleep(500);
	}
}

int ModuleDemo::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
	modulebase demo
)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("modulebase", "controller");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();

	return 0;
}

extern "C" __EXPORT int ModuleDemo_main(int argc, char *argv[])
{
	return ModuleDemo::main(argc, argv);
}
