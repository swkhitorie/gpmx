#include <gpmx/config.h>

#include <mlog.h>

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int shared_counter = 0;
static pthread_spinlock_t spinlock;

#define ITERATIONS 1000000

void *spinlock_dbg1(void *arg)
{
    int i;
    KMRAW("spinlock_dbg1 enter \r\n");

    for (i = 0; i < ITERATIONS; i++) {
        pthread_spin_lock(&spinlock);
        shared_counter++;
        pthread_spin_unlock(&spinlock);
        if (i % 100000 == 0) {
            KMRAW("spinlock_dbg1 loop %d \r\n", i);
        }
    }
    KMRAW("spinlock_dbg1 loop end \r\n");
    return NULL;
}

void *spinlock_dbg2(void *arg)
{
    int i;
    KMRAW("spinlock_dbg2 enter \r\n");
    for (i = 0; i < ITERATIONS; i++) {
        pthread_spin_lock(&spinlock);
        shared_counter++;
        pthread_spin_unlock(&spinlock);
        if (i % 100000 == 0) {
            KMRAW("spinlock_dbg2 loop %d \r\n", i);
        }
    }
    KMRAW("spinlock_dbg2 loop end \r\n");
    return NULL;
}

int klibc_pthread_spinlock_test(int argc, char **argv)
{
    pthread_attr_t attr1, attr2;
    pthread_t thread1, thread2;
    struct sched_param param;
    int ret = 0;

    if (pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
        KMRAW("pthread_spin_init error \r\n");
        return -1;
    }

    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);

    param.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2;
    pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
    pthread_attr_setschedparam(&attr1, &param);
    pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr1, 1280 * sizeof(void *));

    param.sched_priority = sched_get_priority_max(SCHED_FIFO) / 2 + 1;
    pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
    pthread_attr_setschedparam(&attr2, &param);
    pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr2, 1280 * sizeof(void *));

    ret = pthread_create(&thread1, &attr1, spinlock_dbg1, NULL);
    if (ret != 0) {
        KMRAW("pthread_create 1 failed: %d\n", ret);
        return -1;
    }

    ret = pthread_create(&thread2, &attr2, spinlock_dbg2, NULL);
    if (ret != 0) {
        KMRAW("pthread_create 2 failed: %d\n", ret);
        return -1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_spin_destroy(&spinlock);

    int expected = 2 * ITERATIONS;
    if (shared_counter == expected) {
        KMRAW("Test passed: counter = %d (expected %d)\n", shared_counter, expected);
        return 0;
    } else {
        KMRAW("Test failed: counter = %d (expected %d)\n", shared_counter, expected);
        return -1;
    }

    return 0;
}

