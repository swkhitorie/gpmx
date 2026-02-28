#include "kernel_libc_tests.h"

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
    char name[16] = {'\0'};
    float *value = (float *)p;
    int i = 0;
    struct mq_attr attr_node0;

    pthread_setname_np(pthread_self(), "mq_snder");
    pthread_getname_np(pthread_self(), &name[0], 16);

    attr_node0.mq_flags = 0;
    attr_node0.mq_maxmsg = 5;
    attr_node0.mq_msgsize = sizeof(euler_t);
    attr_node0.mq_curmsgs = 0;

    msg_1 = mq_open("/node0", O_RDWR | O_CREAT | O_NONBLOCK, 0, &attr_node0);
    TEST_PRINTF("[%s] msg_1 : 0x%x %d\r\n", &name[0], (uint32_t)msg_1, (uint32_t)msg_1 < 0);

    euler_t tmp = {.pitch = 1.0f, .roll = 2.0f, .yaw = 3.14f,};
    for (;;) {
        tmp.pitch += 0.1f;
        tmp.roll += 0.2f;
        tmp.yaw -= 0.5f;

        mq_send(msg_1, (const char*)&tmp, sizeof(euler_t), 0);
        TEST_PRINTF("[%s] send data: %.3f, %.3f, %.3f\r\n", &name[0], tmp.pitch, tmp.roll, tmp.yaw);
        sleep(500);
    }

    return NULL;
}

void* p5_entry(void *p)
{
    char name[16] = {'\0'};
    float *value = (float *)p;
    int i = 0;

    euler_t tmp_rcv;
    char rcv_array[64];
    mqd_t mq_node0;
    struct mq_attr attr;

    pthread_setname_np(pthread_self(), "mq_rcver");
    pthread_getname_np(pthread_self(), &name[0], 16);

    mq_node0 = mq_open("/node0", O_RDWR | O_NONBLOCK, 0, NULL);
    if (mq_node0 == (mqd_t)-1) {
        TEST_PRINTF("/node0 open failed \r\n");
    } else {
        TEST_PRINTF("/node0 open success \r\n");
    }

    mq_getattr(mq_node0, &attr);

    for (;;) {
        int res = mq_receive(mq_node0, &rcv_array[0], attr.mq_msgsize, NULL);
        if (res == attr.mq_msgsize) {
            memcpy((char *)&tmp_rcv, &rcv_array[0], sizeof(euler_t));
            TEST_PRINTF("[%s] rcv data: %.3f, %.3f, %.3f, %d, %d\r\n", &name[0], tmp_rcv.pitch, tmp_rcv.roll, tmp_rcv.yaw, res, attr.mq_msgsize);
        }

        sleep(50);
    }

    return NULL;
}

int klibc_mq_test(int argc, char **argv)
{
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

    return 0;
}
