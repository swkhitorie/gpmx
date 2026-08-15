#define _GNU_SOURCE
#include <gpmx/config.h>
#include <mlog.h>
#include <string.h>

#include <pthread.h>
#include <time.h>
#include <unistd.h>

static void* usr_test_p(void *p);
int user_test(int argc, char **argv)
{
    int task_id;
    pthread_attr_t attr;
    pthread_t id;
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

    task_id = pthread_create(&id, &attr, &usr_test_p, NULL);

    return 0;
}

void* usr_test_p(void *p)
{
    pthread_setname_np(pthread_self(), "usr_test");

    while(1) {

        sleep(1);
    }

    return NULL;
}
