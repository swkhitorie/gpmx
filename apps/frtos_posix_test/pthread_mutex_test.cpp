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

pthread_test_t p8;
pthread_test_t p9;

pthread_mutex_t lock1;
int critical_val = 3;
void* p8_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;
    strcpy(&name[0], "p8_edited");
    pthread_setname_np(pthread_self(), &name[0]);

    if (!lock1.initialized) {
        pthread_mutex_init(&lock1, NULL);
    }

    for (;;) {
        pthread_mutex_lock(&lock1);
        critical_val++;
        pthread_mutex_unlock(&lock1);

        fprintf(stdout, "[%s] lock after val : %d\r\n", &name[0], critical_val);
        sleep(1000);
    }

    return NULL;
}

void* p9_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;
    strcpy(&name[0], "p9_edited");
    pthread_setname_np(pthread_self(), &name[0]);

    if (!lock1.initialized) {
        pthread_mutex_init(&lock1, NULL);
    }

    for (;;) {
        pthread_mutex_lock(&lock1);
        critical_val--;
        pthread_mutex_unlock(&lock1);

        fprintf(stdout, "[%s] lock after val : %d\r\n", &name[0], critical_val);
        sleep(1000);
        // debug_led_toggle();
    }

    return NULL;
}

int main(void)
{
    board_init();
    board_bsp_init();

    hrt_init();

    {
        int rv;
        p8.param.sched_priority = 6;
        p8.arg = -41.0f;
        pthread_attr_init(&p8.attr);
        pthread_attr_setdetachstate(&p8.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p8.attr, &p8.param);
        pthread_attr_setstacksize(&p8.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p8.id, &p8.attr, &p8_entry, &p8.arg);
        if (rv != 0) {
            fprintf(stdout, "[p8] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    {
        int rv;
        p9.param.sched_priority = 4;
        p9.arg = 0.123f;
        pthread_attr_init(&p9.attr);
        // PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
        pthread_attr_setdetachstate(&p9.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p9.attr, &p9.param);
        pthread_attr_setstacksize(&p9.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p9.id, &p9.attr, &p9_entry, &p9.arg);
        if (rv != 0) {
            fprintf(stdout, "[p9] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    sched_start();
    for (;;);
}
