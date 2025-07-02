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

void tr1_entry(union sigval value)
{
    fprintf(stdout, "[tr1] %.6f sign event \r\n",hrt_absolute_time()/1e6f);
}

int main(void)
{
    board_init();
    board_bsp_init();

    hrt_init();

    //soft timer triggle
    timer_t tr1;
    struct sigevent event;
    pthread_attr_t event_attr;
    struct sched_param event_param = {.sched_priority = 5,};
    pthread_attr_init(&event_attr);
    pthread_attr_setdetachstate(&event_attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedparam(&event_attr, &event_param);
    pthread_attr_setstacksize(&event_attr, 256*sizeof(StackType_t));
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_value.sival_int = 4;
    event.sigev_notify_function = tr1_entry;
    event.sigev_notify_attributes = NULL;
    int res = timer_create(NULL, &event, &tr1);
    fprintf(stdout, "[tr1] timer create: %d\r\n", res);

    struct itimerspec val;
    timer_gettime(tr1, &val);
    fprintf(stdout, "[tr1] get interval: %d %d | %d %d\r\n", 
        val.it_value.tv_sec, val.it_value.tv_nsec,
        val.it_interval.tv_sec, val.it_interval.tv_nsec);

    struct itimerspec new_val;
    clock_gettime(NULL, &new_val.it_value);
    new_val.it_interval.tv_sec = 0;
    new_val.it_interval.tv_nsec = 0;
    new_val.it_value.tv_sec += 3;
    int re2 = timer_settime(tr1, TIMER_ABSTIME, &new_val, &val);
    fprintf(stdout, "[tr1] set interval: %d\r\n", re2);

    sched_start();
    for (;;);
}
