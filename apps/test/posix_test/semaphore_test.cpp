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

pthread_test_t p6;
pthread_test_t p7;

sem_t a_sem;
void* p6_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;
    strcpy(&name[0], "p6_edited");
    pthread_setname_np(pthread_self(), &name[0]);

    sem_init(&a_sem, 0, 1);
    int val;
    sem_getvalue(&a_sem, &val);
    for (;;) {

        fprintf(stdout, "[%s] sem val: %d\r\n", &name[0], val);
        if (0 == sem_wait(&a_sem)) {
            sem_getvalue(&a_sem, &val);
            fprintf(stdout, "[%s] get sem : %d\r\n", &name[0], val);
        }
        sem_post(&a_sem);
        sem_getvalue(&a_sem, &val);
        fprintf(stdout, "[%s] post sem : %d\r\n", &name[0], val);
        sleep(500);
    }

    return NULL;
}

void* p7_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;
    strcpy(&name[0], "p7_edited");
    pthread_setname_np(pthread_self(), &name[0]);

    int val;
    sem_getvalue(&a_sem, &val);
    for (;;) {

        fprintf(stdout, "[%s] sem val: %d\r\n", &name[0], val);
        if (0 == sem_wait(&a_sem)) {
            sem_getvalue(&a_sem, &val);
            fprintf(stdout, "[%s] get sem : %d\r\n", &name[0], val);
        }
        sem_post(&a_sem);
        sem_getvalue(&a_sem, &val);
        fprintf(stdout, "[%s] post sem : %d\r\n", &name[0], val);
        sleep(1000);
        // debug_led_toggle();
    }

    return NULL;
}


int main(void)
{
    board_init();
    hrt_init();

    {
        int rv;
        p6.param.sched_priority = 6;
        p6.arg = -41.0f;
        pthread_attr_init(&p6.attr);
        pthread_attr_setdetachstate(&p6.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p6.attr, &p6.param);
        pthread_attr_setstacksize(&p6.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p6.id, &p6.attr, &p6_entry, &p6.arg);
        if (rv != 0) {
            fprintf(stdout, "[p6] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    {
        int rv;
        p7.param.sched_priority = 4;
        p7.arg = 0.123f;
        pthread_attr_init(&p7.attr);
        // PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
        pthread_attr_setdetachstate(&p7.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p7.attr, &p7.param);
        pthread_attr_setstacksize(&p7.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p7.id, &p7.attr, &p7_entry, &p7.arg);
        if (rv != 0) {
            fprintf(stdout, "[p7] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    sched_start();
    for (;;);
}
