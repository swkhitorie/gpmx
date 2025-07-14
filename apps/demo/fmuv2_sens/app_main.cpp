#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <px4_platform_common/tasks.h>
#include <px4_platform_common/log.h>
#include "drivers/drv_hrt.h"

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

// #include "l3gd20_test.h"

int debug1_main(int argc, char *argv[])
{
	int ret = 0;

    struct timespec cur;
    struct timespec rqtp;
    struct timespec add;
    add.tv_nsec = 20*1e6;
    add.tv_sec = 0;

    for(;;) {

        printf("test debug1 \r\n");

        board_debug();

        clock_gettime(0, &cur);
        utils_timespec_add(&cur, &add, &rqtp);
        clock_nanosleep(0, TIMER_ABSTIME, &rqtp, NULL);
    }

	PX4_INFO("debug1_main exit");
	return ret;
}

void b_start(void *p)
{
    board_bsp_init();
    hrt_init();

    px4_task_spawn_cmd("debug_1", 
        SCHED_DEFAULT,
        SCHED_PRIORITY_MIN + 2, 
        1024,
        debug1_main,
        (char *const *)NULL);

    vTaskDelete(NULL);
}

int main(int argc, char *argv[])
{
    board_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(b_start, "b_start", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}

