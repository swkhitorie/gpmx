#include <board_config.h>
#include <drivers/drv_hrt.h>

#include <pthread.h>

#include "ModuleDemo.hpp"

#define MODULE_NAME "logger"

int px4_app1_main(int argc, char *argv[])
{
	int ret = 0;

    for(;;) {
		sleep(100);
        PX4_INFO("px4 app111 running %.5f", hrt_absolute_time()/1e6f);
    }

	PX4_INFO("px4 app1 exit");
	return ret;
}

// see https://github.com/PX4/PX4-Autopilot/tree/main/src/templates/template_module

char cmd[] = "start";
char cmd2[] = "ModuleDemo";
char *t_argv_list[10];

int main(void)
{
    board_init();
    board_bsp_init();

    hrt_init();

    t_argv_list[0] = &cmd2[0];
    t_argv_list[1] = &cmd[0];

    ModuleDemo::main(2, t_argv_list);

    px4_task_t taskid1 = px4_task_spawn_cmd( "px4_app1", 
                                            SCHED_DEFAULT, SCHED_PRIORITY_MIN + 3, 
                                            4096, px4_app1_main, (char *const *)NULL);

    sched_start();
    for (;;);
}
