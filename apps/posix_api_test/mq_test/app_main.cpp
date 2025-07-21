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
pthread_test_t p4;
pthread_test_t p5;

typedef struct __euler {
    float pitch;
    float roll;
    float yaw;
} euler_t;

mqd_t msg_1;

void* p4_entry(void *p)
{
    float *value = (float *)p;
    int i = 0;

    pthread_setname_np(pthread_self(), "p4_edited");

    msg_1 = mq_open("/node0", O_RDWR | O_CREAT | O_NONBLOCK, 0, NULL);
    printf("[%s] msg_1 : %d %d\r\n", &name[0], msg_1, msg_1 < 0);

    euler_t tmp = {.pitch = 1.0f, .roll = 2.0f, .yaw = 3.14f,};
    for (;;) {
        tmp.pitch += 0.1f;
        tmp.roll += 0.2f;
        tmp.yaw -= 0.5f;

        mq_send(msg_1, (const char*)&tmp, sizeof(euler_t), 0);
        printf("[%s] send data: %.3f, %.3f, %.3f\r\n", &name[0], tmp.pitch, tmp.roll, tmp.yaw);
        sleep(500);
    }

    return NULL;
}

void* p5_entry(void *p)
{
    float *value = (float *)p;
    int i = 0;

    pthread_setname_np(pthread_self(), "p5_edited");

    euler_t tmp_rcv;
    char rcv_array[64];
    struct mq_attr attr;
    mq_getattr(msg_1, &attr);

    for (;;) {
        int res = mq_receive(msg_1, &rcv_array[0], attr.mq_msgsize, NULL);
        memcpy((char *)&tmp_rcv, &rcv_array[0], sizeof(euler_t));
        printf("[%s] rcv data: %.3f, %.3f, %.3f, %d\r\n", &name[0], tmp_rcv.pitch, tmp_rcv.roll, tmp_rcv.yaw, res);
        sleep(1000);
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
        p4.param.sched_priority = 6;
        p4.arg = -41.0f;
        pthread_attr_init(&p4.attr);
        pthread_attr_setdetachstate(&p4.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p4.attr, &p4.param);
        pthread_attr_setstacksize(&p4.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p4.id, &p4.attr, &p4_entry, &p4.arg);
    }

    {
        int rv;
        p5.param.sched_priority = 4;
        p5.arg = 0.123f;
        pthread_attr_init(&p5.attr);
        pthread_attr_setdetachstate(&p5.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p5.attr, &p5.param);
        pthread_attr_setstacksize(&p5.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p5.id, &p5.attr, &p5_entry, &p5.arg);
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

