#include <board_config.h>
#include <stdio.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include <drivers/drv_hrt.h>
#include <pthread.h>
#include "cdev_example.h"

typedef struct __pthread_test{
    pthread_attr_t attr;
    pthread_t id;
    float arg;
    struct sched_param param;
} pthread_test_t;
pthread_test_t g_p;

void* g_start(void *p)
{
    board_bsp_init();
    hrt_init();

    pthread_setname_np(pthread_self(), "g_start");

    printf("cdevtest\n");
    CDevExample cdevtest;
    cdevtest.main();
    printf("goodbye\n");

    return NULL;
}

int main(void)
{
    board_init();

    g_p.param.sched_priority = 1;
    pthread_attr_init(&g_p.attr);
    pthread_attr_setdetachstate(&g_p.attr, PTHREAD_CREATE_DETACHED); //PTHREAD_CREATE_JOINABLE
    pthread_attr_setschedparam(&g_p.attr, &g_p.param);
    pthread_attr_setstacksize(&g_p.attr, 512*sizeof(StackType_t));
    int rv = pthread_create(&g_p.id, &g_p.attr, &g_start, &g_p.arg);

    sched_start();
    for (;;);
}
