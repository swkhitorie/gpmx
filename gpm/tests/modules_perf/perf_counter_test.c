#define _GNU_SOURCE

#include <gpmx/config.h>

#include "perf_counter.h"
#include "perf_counter_test.h"

#include "driver/drv_hrt.h"

#include <pthread.h>
#include <time.h>
#include <unistd.h>

static perf_counter_t _elaped_perf;

static void* perf_elasped_test(void *p)
{
    pthread_setname_np(pthread_self(), "perf_elapsed");
    _elaped_perf = perf_alloc(PC_ELAPSED, "cycle time");
    int counter = 0;

    for (;;) {
		perf_begin(_elaped_perf);

        sleep(100);

		perf_end(_elaped_perf);

        counter++;
        if (counter % 10 == 0) {
            perf_print_counter(_elaped_perf);
        }
    }

    perf_free(_elaped_perf);
    return NULL;
}

int perf_counter_test(int argc, char **argv)
{
    int task_id;
    pthread_attr_t attr;
    pthread_t id;

    hrt_init();

    pthread_attr_init(&attr);

#if 1
    struct sched_param attr_param;
    attr_param.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    pthread_attr_setschedparam(&attr, &attr_param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 512*sizeof(void *));
#else
    attr.schedparam.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    attr.detachstate = PTHREAD_CREATE_JOINABLE;
    attr.stacksize = 512 * 4;
#endif

    task_id = pthread_create(&id, &attr, &perf_elasped_test, NULL);

    return 0;
}
