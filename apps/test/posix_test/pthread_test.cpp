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
#include <utils.h>
#include <pthread.h>

typedef struct __pthread_test{
    pthread_attr_t attr;
    pthread_t id;
    float arg;
    struct sched_param param;
} pthread_test_t;

pthread_test_t p1;
pthread_test_t p2;

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
    for (; i < 3; i++) {
        fprintf(stdout, "[%s] %.6f p1 tag, val: %.5f, %d\r\n", name, hrt_absolute_time()/1e6f,
                                    *value, i);
        sleep(1000); // sleep 1s
    }
    fprintf(stdout, "[%s] %.6f p1 end\r\n", name, hrt_absolute_time()/1e6f);
    return NULL;
}

void* p2_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;
    pthread_getname_np(pthread_self(), &name[0], 16);
    fprintf(stdout, "[%s] %.6f p2 start\r\n", name, hrt_absolute_time()/1e6f);
    memset(&name[0], 0, 16);
    strcpy(&name[0], "p2_edited");
    pthread_setname_np(pthread_self(), &name[0]);
    for (; i < 15; i++) {
        fprintf(stdout, "[%s] %.6f p2 tag, val: %.5f, %d\r\n", name, hrt_absolute_time()/1e6f,
                                    *value, i);
        sleep(2000); // sleep 2s
    }
    fprintf(stdout, "[%s] %.6f p2 end\r\n", name, hrt_absolute_time()/1e6f);
    return NULL;
}

int main(void)
{
    board_init();
    hrt_init();

    {
        int rv;
        p1.param.sched_priority = 4;
        p1.arg = 3.425f;
        pthread_attr_init(&p1.attr);
        pthread_attr_setdetachstate(&p1.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p1.attr, &p1.param);
        pthread_attr_setstacksize(&p1.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p1.id, &p1.attr, &p1_entry, &p1.arg);
        if (rv != 0) {
            fprintf(stdout, "[p1] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    {
        int rv;
        p2.param.sched_priority = 6;
        p2.arg = 0.123f;
        pthread_attr_init(&p2.attr);
        // PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
        pthread_attr_setdetachstate(&p2.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p2.attr, &p2.param);
        pthread_attr_setstacksize(&p2.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p2.id, &p2.attr, &p2_entry, &p2.arg);
        if (rv != 0) {
            fprintf(stdout, "[p2] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    sched_start();
    for (;;);
}
