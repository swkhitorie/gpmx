#include <board_config.h>
#include <drivers/drv_hrt.h>

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
#include <pthread.h>

#include <lib/perf/perf_counter.h>
#include <px4_platform_common/log.h>

#define MODULE_NAME "logger"

typedef struct __pthread_test{
    pthread_attr_t attr;
    pthread_t id;
    float arg;
    struct sched_param param;
} pthread_test_t;

pthread_test_t p1;

perf_counter_t test_perf;

void* p1_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;

    pthread_getname_np(pthread_self(), &name[0], 16);
    fprintf(stdout, "[%s] %.6f p1 start\r\n", name, hrt_absolute_time()/1e6f);
    memset(&name[0], 0, 16);
    strcpy(&name[0], "p1_edited");
    pthread_setname_np(pthread_self(), &name[0]);

    perf_begin(test_perf);
    for (; i < 3; i++) {

        PX4_INFO("aaa hhl \r\n");
        sleep(1000); // sleep 1s
    }
    perf_end(test_perf);
    perf_print_counter(test_perf);

    return NULL;
}

int main(void)
{
    board_init();
    board_bsp_init();

    hrt_init();

    test_perf = perf_alloc(PC_ELAPSED, "test");

    int rv;
    p1.param.sched_priority = 4;
    pthread_attr_init(&p1.attr);
    pthread_attr_setdetachstate(&p1.attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedparam(&p1.attr, &p1.param);
    pthread_attr_setstacksize(&p1.attr, 512*sizeof(StackType_t));
    rv = pthread_create(&p1.id, &p1.attr, &p1_entry, &p1.arg);
    if (rv != 0) {
        fprintf(stdout, "[p1] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
    }

    sched_start();
    for (;;);
}
