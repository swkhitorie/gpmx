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
pthread_test_t g_p;
pthread_test_t p6;
pthread_test_t p7;

sem_t a_sem;
void* p6_entry(void *p)
{
    float *value = (float *)p;
    int i = 0;

    pthread_setname_np(pthread_self(), "p6_edited");

    sem_init(&a_sem, 0, 1);
    int val;
    sem_getvalue(&a_sem, &val);
    for (;;) {

        printf("[%s] sem val: %d\r\n", "p6_edited", val);
        if (0 == sem_wait(&a_sem)) {
            sem_getvalue(&a_sem, &val);
            printf("[%s] get sem : %d\r\n", "p6_edited", val);
        }
        sem_post(&a_sem);
        sem_getvalue(&a_sem, &val);
        printf("[%s] post sem : %d\r\n", "p6_edited", val);
        sleep(500);
    }

    return NULL;
}

void* p7_entry(void *p)
{
    float *value = (float *)p;
    int i = 0;

    pthread_setname_np(pthread_self(), "p7_edited");

    int val;
    sem_getvalue(&a_sem, &val);
    for (;;) {

        printf("[%s] sem val: %d\r\n", "p7_edited", val);
        if (0 == sem_wait(&a_sem)) {
            sem_getvalue(&a_sem, &val);
            printf("[%s] get sem : %d\r\n", "p7_edited", val);
        }
        sem_post(&a_sem);
        sem_getvalue(&a_sem, &val);
        printf("[%s] post sem : %d\r\n", "p7_edited", val);
        sleep(1000);
        // debug_led_toggle();
    }

    return NULL;
}

void* g_start(void *p)
{
    board_bsp_init();
    hrt_init();

    pthread_setname_np(pthread_self(), "g_start");

    {
        int rv;
        p6.param.sched_priority = 6;
        p6.arg = -41.0f;
        pthread_attr_init(&p6.attr);
        pthread_attr_setdetachstate(&p6.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p6.attr, &p6.param);
        pthread_attr_setstacksize(&p6.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p6.id, &p6.attr, &p6_entry, &p6.arg);
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
    }

    return NULL;
}

int main(void)
{
    board_init();
    // taskENTER_CRITICAL();

    g_p.param.sched_priority = 1;
    pthread_attr_init(&g_p.attr);
    pthread_attr_setdetachstate(&g_p.attr, PTHREAD_CREATE_DETACHED); //PTHREAD_CREATE_JOINABLE
    pthread_attr_setschedparam(&g_p.attr, &g_p.param);
    pthread_attr_setstacksize(&g_p.attr, 512*sizeof(StackType_t));
    int rv = pthread_create(&g_p.id, &g_p.attr, &g_start, &g_p.arg);

    // taskEXIT_CRITICAL();
    sched_start();
    for (;;);
}
