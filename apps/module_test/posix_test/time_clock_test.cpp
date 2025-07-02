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

// 1:Delay 2:Delay Until
#define POSIX_DELAY_METHOD   (2)

typedef struct __pthread_test{
    pthread_attr_t attr;
    pthread_t id;
    float arg;
    struct sched_param param;
} pthread_test_t;

pthread_test_t p3;

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
    z1.tv_nsec = 4000 * 1000 * 1000;
    sprintf(d1, "hello world\r\n");
    fprintf(stdout, "strlen : %d\r\n", utils_strlen(d1, 30));
    fprintf(stdout, "x1 valid:%d, y1 valid:%d, z1 valid:%d\r\n",
                        utils_validtimespec(&x1), utils_validtimespec(&y1), utils_validtimespec(&z1));
    fprintf(stdout, "x1>y1?:%d, y1>x1?:%d \r\n", utils_timespec_compare(&x1, &y1), utils_timespec_compare(&y1, &x1));
    res = utils_timespec_add(&x1, &y1, &z1);
    fprintf(stdout, "x1+y1:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_subtract(&x1, &y1, &z1);
    fprintf(stdout, "x1-y1:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_addnanoseconds(&x1, 350*1000, &z1);
    fprintf(stdout, "x1+30*1000*1000:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_toticks(&x1, &a1);
    fprintf(stdout, "x1 to ticks:%d | %d\r\n", a1, res);
    utils_nanoseconds_totimespec(1 * 1000 * 1000 * 1000 + 3 * 1000, &z1);
    fprintf(stdout, "1000000000+3000:%d,%d | %d\r\n", z1.tv_sec, z1.tv_nsec, res);
    res = utils_timespec_todeltaticks(&x1, &y1, &a1);
    fprintf(stdout, "delta ticks :%d | %d\r\n", a1, res);
}


static char debug_str1[1024];
void* p3_entry(void *p)
{
    struct timespec rqtp;
    struct timespec x1;
#if POSIX_DELAY_METHOD == 1
    rqtp.tv_sec = 1;
    rqtp.tv_nsec = 0;
#endif
    for (;;) {
        // debug_led_toggle();
        // vTaskList(&debug_str1[0]);
        // printf("%s\r\n", debug_str1);
        vTaskGetRunTimeStats(&debug_str1[0]);
        printf("%s\r\n", debug_str1);
        printf("%.6f %.6f %.3f\r\n", portGET_RUN_TIME_COUNTER_VALUE()/1e6f, hrt_absolute_time()/1e6f, HAL_GetTick()/1e3f);
        //utils_fr_posix_debug();
        clock_gettime(0, &x1);
        fprintf(stdout, "clock gettime: %d %d\r\n", x1.tv_sec, x1.tv_nsec);
#if POSIX_DELAY_METHOD == 2
        clock_gettime(0, &rqtp);
        rqtp.tv_sec += 1;
        int re = clock_nanosleep(0, TIMER_ABSTIME, &rqtp, NULL);
#endif
#if POSIX_DELAY_METHOD == 1
        nanosleep(&rqtp, NULL);
#endif
    }
}

int main(void)
{
    board_init();
    board_bsp_init();

    hrt_init();

    {
        int rv;
        p3.param.sched_priority = 6;
        p3.arg = 0.123f;
        pthread_attr_init(&p3.attr);
        // PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
        pthread_attr_setdetachstate(&p3.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p3.attr, &p3.param);
        pthread_attr_setstacksize(&p3.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p3.id, &p3.attr, &p3_entry, &p3.arg);
        if (rv != 0) {
            fprintf(stdout, "[p3] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    sched_start();
    for (;;);
}
