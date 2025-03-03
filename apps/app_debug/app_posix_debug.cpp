#include "./app_posix_debug.h"

typedef struct __pthread_test{
    pthread_attr_t attr;
    pthread_t id;
    float arg;
    struct sched_param param;
} pthread_test_t;

typedef struct __euler {
    float pitch;
    float roll;
    float yaw;
} euler_t;

pthread_test_t p1;
pthread_test_t p2;
pthread_test_t p3;
pthread_test_t p4;
pthread_test_t p5;
pthread_test_t p6;
pthread_test_t p7;
pthread_test_t p8;
pthread_test_t p9;
mqd_t msg_1;


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

void time_clock_fr_debug()
{
    struct timespec x1;
    clock_gettime(0, &x1);
    fprintf(stdout, "clock gettime: %d %d\r\n", x1.tv_sec, x1.tv_nsec);
}




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
        sleep(1000); // sleep 2s
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
        sleep(1000);
        debug_led_toggle();
    }
    fprintf(stdout, "[%s] %.6f p2 end\r\n", name, hrt_absolute_time()/1e6f);
    return NULL;
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
        debug_led_toggle();
        vTaskList(&debug_str1[0]);
        printf("%s\r\n", debug_str1);
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

void* p4_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;
    strcpy(&name[0], "p4_edited");
    pthread_setname_np(pthread_self(), &name[0]);

    msg_1 = mq_open("/node0", O_RDWR | O_CREAT | O_NONBLOCK, 0, NULL);
    fprintf(stdout, "[%s] msg_1 : %d %d\r\n", &name[0], msg_1, msg_1 < 0);

    euler_t tmp = {.pitch = 1.0f, .roll = 2.0f, .yaw = 3.14f,};
    for (;;) {
        tmp.pitch += 0.1f;
        tmp.roll += 0.2f;
        tmp.yaw -= 0.5f;

        mq_send(msg_1, (const char*)&tmp, sizeof(euler_t), 0);
        fprintf(stdout, "[%s] send data: %.3f, %.3f, %.3f\r\n", &name[0], tmp.pitch, tmp.roll, tmp.yaw);
        sleep(500);
    }

    return NULL;
}

void* p5_entry(void *p)
{
    float *value = (float *)p;
    char name[16] = {'\0'};
    int i = 0;
    strcpy(&name[0], "p5_edited");
    pthread_setname_np(pthread_self(), &name[0]);

    euler_t tmp_rcv;
    char rcv_array[64];
    struct mq_attr attr;
    mq_getattr(msg_1, &attr);

    for (;;) {
        int res = mq_receive(msg_1, &rcv_array[0], attr.mq_msgsize, NULL);
        memcpy((char *)&tmp_rcv, &rcv_array[0], sizeof(euler_t));
        fprintf(stdout, "[%s] rcv data: %.3f, %.3f, %.3f, %d\r\n", &name[0], tmp_rcv.pitch, tmp_rcv.roll, tmp_rcv.yaw, res);
        sleep(1000);
        debug_led_toggle();
    }

    return NULL;
}

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
        debug_led_toggle();
    }

    return NULL;
}

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
        debug_led_toggle();
    }

    return NULL;
}

void tr1_entry(union sigval value)
{
    fprintf(stdout, "[tr1] %.6f sign event \r\n",hrt_absolute_time()/1e6f);
}



void app_posix_freertos_debug_init()
{

#if POSIX_TEST_ITEM == 1
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
#endif

#if POSIX_TEST_ITEM == 2
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
#endif

#if POSIX_TEST_ITEM == 3
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
#endif


#if POSIX_TEST_ITEM == 4
    {
        int rv;
        p4.param.sched_priority = 6;
        p4.arg = -41.0f;
        pthread_attr_init(&p4.attr);
        pthread_attr_setdetachstate(&p4.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p4.attr, &p4.param);
        pthread_attr_setstacksize(&p4.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p4.id, &p4.attr, &p4_entry, &p4.arg);
        if (rv != 0) {
            fprintf(stdout, "[p4] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }

    {
        int rv;
        p5.param.sched_priority = 4;
        p5.arg = 0.123f;
        pthread_attr_init(&p5.attr);
        // PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
        pthread_attr_setdetachstate(&p5.attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setschedparam(&p5.attr, &p5.param);
        pthread_attr_setstacksize(&p5.attr, 512*sizeof(StackType_t));
        rv = pthread_create(&p5.id, &p5.attr, &p5_entry, &p5.arg);
        if (rv != 0) {
            fprintf(stdout, "[p5] %.6f create pthread failed\r\n",hrt_absolute_time()/1e6f);
        }
    }
#endif


#if POSIX_TEST_ITEM == 5
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
#endif


#if POSIX_TEST_ITEM == 6
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
#endif

}


