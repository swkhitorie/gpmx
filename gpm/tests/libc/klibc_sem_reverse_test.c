#include <gpmx/config.h>
#include <mlog.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

#include <driver/drv_hrt.h>

static sem_t rev_sem;

static volatile uint64_t high_wait_start = 0;
static volatile uint64_t high_acquire_time = 0;

static void *low_prio_task(void *p)
{
    (void)p;
    char name[16] = {'\0'};
    pthread_getname_np(pthread_self(), &name[0], 16);

    sem_wait(&rev_sem);
    uint64_t t_start = hrt_absolute_time();
    KMRAW("[%s][%llu us] Low-prio : acquired sem, working 500 ms...\n", name, t_start);

    /* wait 500ms */
    while (hrt_absolute_time() - t_start < 500000UL) {
        uint64_t t0 = hrt_absolute_time();
        while (hrt_absolute_time() - t0 < 10000) {}
        sched_yield();
    }

    uint64_t t_end = hrt_absolute_time();
    KMRAW("[%s][%llu us] Low-prio : releasing sem\n", name, t_end);
    sem_post(&rev_sem);
    return NULL;
}

static void *mid_prio_task(void *p)
{
    char name[16] = {'\0'};
    (void)p;
    pthread_getname_np(pthread_self(), &name[0], 16);

    uint64_t t_start = hrt_absolute_time();
    KMRAW("[%s][%llu us] Mid-prio : start busy loop (3 s)\n", name, t_start);

    /* wait 3s */
    while (hrt_absolute_time() - t_start < 3000000UL) {
        uint64_t t0 = hrt_absolute_time();
        while (hrt_absolute_time() - t0 < 10000) {}
        sched_yield();
    }

    uint64_t t_end = hrt_absolute_time();
    KMRAW("[%s][%llu us] Mid-prio : finished\n", name, t_end);
    return NULL;
}

static void *high_prio_task(void *p)
{
    (void)p;
    char name[16] = {'\0'};
    pthread_getname_np(pthread_self(), &name[0], 16);

    /* Waiting for low priority tasks to obtain semaphore first */
    // 200ms，Ensure that low priority has obtained the lock and entered the busy state
    usleep(200 * 1000);  

    high_wait_start = hrt_absolute_time();
    KMRAW("[%s][%llu us] High-prio: requesting sem...\n", name, high_wait_start);

    sem_wait(&rev_sem);

    high_acquire_time = hrt_absolute_time();
    uint64_t waited = high_acquire_time - high_wait_start;
    KMRAW("[%s][%llu us] High-prio: acquired sem, waited %llu us\n",
          name, high_acquire_time, waited);

    sem_post(&rev_sem);
    return NULL;
}

static uint64_t run_test_round(int protocol)
{
    pthread_t low, mid, high;
    struct sched_param param;
    pthread_attr_t attr;
    uint64_t wait_time;

    sem_init(&rev_sem, 0, 1);
    sem_setprotocol(&rev_sem, protocol);

    high_wait_start = 0;
    high_acquire_time = 0;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 2048);

    param.sched_priority = 5;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&low, &attr, low_prio_task, NULL);

    /* give up the CPU to ensure that low priority tasks 
       start executing and obtaining semaphores */

    usleep(50 * 1000);

    param.sched_priority = 7;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&mid, &attr, mid_prio_task, NULL);

    param.sched_priority = 9;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&high, &attr, high_prio_task, NULL);


    pthread_join(high, NULL);
    wait_time = high_acquire_time - high_wait_start;

    pthread_join(mid, NULL);
    pthread_join(low, NULL);

    sem_destroy(&rev_sem);
    return wait_time;
}

int klibc_sem_reverse_test(int argc, char **argv)
{
    uint64_t time_none, time_inherit;
    pthread_setname_np(pthread_self(), "sem_test2");

    KMRAW("\n=== Priority Inversion Test ===\n");

    KMRAW("[Test 1] Protocol = SEM_PRIO_NONE\n");
    time_none = run_test_round(SEM_PRIO_NONE);

    KMRAW("\n[Test 2] Protocol = SEM_PRIO_INHERIT\n");
    time_inherit = run_test_round(SEM_PRIO_INHERIT);

    KMRAW("\n=== Results ===\n");
    KMRAW("Without inherit: %llu us\n", time_none);
    KMRAW("With inherit   : %llu us\n", time_inherit);

    if (time_inherit < time_none / 3 && time_inherit < 1000000) {
        KMRAW("Priority inheritance is effective.\n");
        KMRAW("TEST PASSED\n");
    } else {
        KMRAW("Priority inheritance NOT effective or test unreliable.\n");
        KMRAW("TEST FAILED\n");
    }

    return 0;
}
