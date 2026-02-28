#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sched.h>
#include <sdqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <utils.h>
#include <pthread.h>

#include <px4_platform_common/module.h>
#include <lib/perf/perf_counter.h>

class ModuleDemo : public ModuleBase<ModuleDemo>
{
public:
	ModuleDemo();
	~ModuleDemo() override;

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	/** @see ModuleBase */
	static ModuleDemo *instantiate(int argc, char *argv[]);

	/** @see ModuleBase */
	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	/** @see ModuleBase::run() */
	void run() override;

	/** @see ModuleBase::print_status() */
	int print_status() override;

private:

	perf_counter_t _cycle_perf;
};

/*
test example

#include <board_config.h>
#include <drivers/drv_hrt.h>

#include <pthread.h>

#include "ModuleDemo.hpp"

#define MODULE_NAME "app1"

char cmd[] = "start";
char cmd3[] = "status";
char cmd2[] = "ModuleDemo";
char *t_argv_list[10];

int px4_app1_main(int argc, char *argv[])
{
	int ret = 0;

    t_argv_list[0] = &cmd2[0];
    t_argv_list[1] = &cmd3[0];

    for(;;) {
		sleep(250);
        PX4_INFO("px4 app1 running %.5f", hrt_absolute_time()/1e6f);
        ModuleDemo::main(2, t_argv_list);
    }

	PX4_INFO("px4 app1 exit");
	return ret;
}

// see https://github.com/PX4/PX4-Autopilot/tree/main/src/templates/template_module
void g_start(void *p)
{
    t_argv_list[0] = &cmd2[0];
    t_argv_list[1] = &cmd[0];

    ModuleDemo::main(2, t_argv_list);

    px4_task_t taskid1 = px4_task_spawn_cmd( "px4_app1", 
                                            SCHED_DEFAULT, SCHED_PRIORITY_MIN + 3, 
                                            4096, px4_app1_main, (char *const *)NULL);

}

 */