#include <board_config.h>
#include <drivers/drv_hrt.h>

#include <pthread.h>

#include "ScheduledWorkItemDemo.hpp"

#define MODULE_NAME "logger"

char cmd[] = "start";
char cmd2[] = "SWorkItemExample";
char *t_argv_list[10];

static char debug_2[512];
int px4_app1_main(int argc, char *argv[])
{
	int ret = 0;
    bool sworkItem_init = false;
    for(;;) {
		sleep(100);

        if (!sworkItem_init && hrt_absolute_time()/1e6f > 8.0f) {
            sworkItem_init = true;
            int ret = SWorkItemExample::main(2, t_argv_list);
            PX4_INFO("SWorkItemEXample start ret: %d \r\n", ret);
        }

        // vTaskList(&debug_2[0]);
        // printf("%s \r\n", debug_2);
        PX4_INFO("px4 app111 running %.5f %d ", hrt_absolute_time()/1e6f, SWorkItemExample::is_running());
    }

	PX4_INFO("px4 app1 exit");
	return ret;
}

// see https://github.com/PX4/PX4-Autopilot/tree/main/src/examples/work_item

void* g_start(void *p)
{
    board_bsp_init();
    hrt_init();

    pthread_setname_np(pthread_self(), "g_start");

    t_argv_list[0] = &cmd2[0];
    t_argv_list[1] = &cmd[0];

    px4::WorkQueueManagerStart();
    PX4_INFO("workqueue started \r\n");

    px4_task_t taskid1 = px4_task_spawn_cmd( "px4_app1", 
                                            SCHED_DEFAULT, SCHED_PRIORITY_MIN + 3, 
                                            4096, px4_app1_main, (char *const *)NULL);

    return NULL;
}

int main(void)
{
    board_init();

    pthread_t tid;
    pthread_attr_t tmpattr = {
        .stackaddr = NULL,
        .stacksize = 2048*sizeof(StackType_t),
        .inheritsched = SCHED_FIFO,
        .schedparam = {
            .sched_priority = 1,
        },
        .detachstate = PTHREAD_CREATE_DETACHED,
    };
    int rv = pthread_create(&tid, &tmpattr, &g_start, NULL);

    sched_start();
    for (;;);
}

