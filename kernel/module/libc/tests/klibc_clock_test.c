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

// 1:Delay 2:Delay Until
#define POSIX_DELAY_METHOD   (2)

typedef struct __pthread_test{
    pthread_attr_t attr;
    pthread_t id;
    float arg;
    struct sched_param param;
} pthread_test_t;

pthread_test_t clk_p;

void utils_fr_posix_debug()
{
    int res = 0;
    int pos = 0;
    char d1[20];  /* can not initialized with char d1[20] = "str"; */
    struct timespec x1;
    struct timespec y1;
    struct timespec z1;
    TickType_t a1;
    x1.tv_sec = 5;
    x1.tv_nsec = 30 * 1000 * 1000;
    y1.tv_sec = 4;
    y1.tv_nsec = 700 * 1000;
    z1.tv_sec = 2;
    z1.tv_nsec = 400 * 1000 * 1000;
    sprintf(d1, "hello world\r\n");
    TEST_PRINTF("strlen : %d\r\n", utils_strlen(d1, 30));
    TEST_PRINTF("x1 valid:%d, y1 valid:%d, z1 valid:%d\r\n",
                        utils_validtimespec(&x1), utils_validtimespec(&y1), utils_validtimespec(&z1));
    TEST_PRINTF("x1>y1?:%d, y1>x1?:%d \r\n", utils_timespec_compare(&x1, &y1), utils_timespec_compare(&y1, &x1));
    res = utils_timespec_add(&x1, &y1, &z1);
    TEST_PRINTF("x1+y1:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_subtract(&x1, &y1, &z1);
    TEST_PRINTF("x1-y1:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_addnanoseconds(&x1, 350*1000, &z1);
    TEST_PRINTF("x1+30*1000*1000:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_toticks(&x1, &a1);
    TEST_PRINTF("x1 to ticks:%d | %d\r\n", a1, res);
    utils_nanoseconds_totimespec(1 * 1000 * 1000 * 1000 + 3 * 1000, &z1);
    TEST_PRINTF("1000000000+3000:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_todeltaticks(&x1, &y1, &a1);
    TEST_PRINTF("delta ticks :%d | %d\r\n", a1, res);
}


static char debug_str1[1024];
void* clk_entry(void *p)
{
    char name[16] = {'\0'};
    struct timespec rqtp;
    struct timespec x1;
#if POSIX_DELAY_METHOD == 1
    rqtp.tv_sec = 1;
    rqtp.tv_nsec = 0;
#endif

    pthread_setname_np(pthread_self(), "clk_test");
    pthread_getname_np(pthread_self(), &name[0], 16);

    for (;;) {

        TEST_PRINTF("[%s] %.3f\r\n", &name[0], board_get_time()/1e3f);

        clock_gettime(0, &x1);
        TEST_PRINTF("[%s] clock gettime: %d %d\r\n", &name[0], x1.tv_sec, x1.tv_nsec);
#if POSIX_DELAY_METHOD == 2
        clock_gettime(0, &rqtp);
        rqtp.tv_sec += 1;
        // if add nsec:
        //     call utils_timespec_add(&cur, &add, &rqtp);
        int re = clock_nanosleep(0, TIMER_ABSTIME, &rqtp, NULL);
#endif
#if POSIX_DELAY_METHOD == 1
        nanosleep(&rqtp, NULL);
#endif
    }
}

int klibc_clock_test(int argc, char **argv)
{
    clk_p.param.sched_priority = 3;
    pthread_attr_init(&clk_p.attr);
    pthread_attr_setdetachstate(&clk_p.attr, PTHREAD_CREATE_JOINABLE); //PTHREAD_CREATE_DETACHED
    pthread_attr_setschedparam(&clk_p.attr, &clk_p.param);
    pthread_attr_setstacksize(&clk_p.attr, 512*sizeof(StackType_t));
    int rv = pthread_create(&clk_p.id, &clk_p.attr, &clk_entry, &clk_p.arg);

    return 0;
}
