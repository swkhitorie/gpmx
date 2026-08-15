#include <gpmx/config.h>

#include <mlog.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 5

static pthread_barrier_t barrier;
static int counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread_func(void* arg)
{
    int id = *(int*)arg;
    int rc;

    KMRAW("Thread %d: before barrier\n", id);
    usleep(100000 * id);

    rc = pthread_barrier_wait(&barrier);
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
        KMRAW("Thread %d: barrier wait failed (rc=%d)\n", id, rc);
        return NULL;
    }

    KMRAW("Thread %d: after barrier\n", id);

    pthread_mutex_lock(&counter_mutex);
    counter++;
    pthread_mutex_unlock(&counter_mutex);

    return NULL;
}

int klibc_pthread_barrier_test(int argc, char **argv)
{
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int i, rc;

    rc = pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    if (rc != 0) {
        KMRAW("pthread_barrier_init failed \r\n");
        return -1;
    }

    for (i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        rc = pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
        if (rc != 0) {
            KMRAW("pthread_create failed\r\n");
            return -1;
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    KMRAW("All threads finished, counter = %d (expected %d)\n", counter, NUM_THREADS);
    if (counter == NUM_THREADS) {
        KMRAW("Test passed.\n");
        return 0;
    } else {
        KMRAW("Test failed.\n");
        return 1;
    }
}
