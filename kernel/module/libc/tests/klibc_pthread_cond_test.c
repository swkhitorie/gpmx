#include "kernel_libc_tests.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_WORKERS 3

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
static int condition_met = 0;
static int waiting_count = 0;
static int awakened_count = 0;

static pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER;
static int ready_count = 0;

void* worker(void* arg)
{
    int id = *(int*)arg;

    pthread_mutex_lock(&barrier_mutex);
    ready_count++;
    pthread_mutex_unlock(&barrier_mutex);

    pthread_mutex_lock(&mutex);
    waiting_count++;
    TEST_PRINTF("Worker %d: waiting... (waiting_count=%d)\n", id, waiting_count);
    while (!condition_met) {
        pthread_cond_wait(&cond, &mutex);
    }

    awakened_count++;
    TEST_PRINTF("Worker %d: awakened (awakened_count=%d)\n", id, awakened_count);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int klibc_pthread_cond_test(int argc, char **argv)
{
    pthread_t threads[NUM_WORKERS];
    int ids[NUM_WORKERS];
    int i;

    TEST_PRINTF("=== Multiple waiters test with broadcast ===\n");

    for (i = 0; i < NUM_WORKERS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    while (1) {
        pthread_mutex_lock(&barrier_mutex);
        int r = ready_count;
        pthread_mutex_unlock(&barrier_mutex);
        if (r == NUM_WORKERS) break;
        usleep(10000);
    }

    usleep(100000);

    TEST_PRINTF("Main: setting condition and broadcasting...\n");

#if defined(CONFIG_FREERTOS_ENABLE)
    // NUM_WORKERS of pthread state -> SUSPENDED
    static char pstr2[512];
    vTaskList(pstr2);
    TEST_PRINTF("\r\n%s\r\n", pstr2);
#endif

    pthread_mutex_lock(&mutex);
    condition_met = 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for (i = 0; i < NUM_WORKERS; i++) {
        pthread_join(threads[i], NULL);
    }

    TEST_PRINTF("Main: awakened_count = %d (expected %d)\n", awakened_count, NUM_WORKERS);
    if (awakened_count == NUM_WORKERS) {
        TEST_PRINTF("=== TEST PASSED ===\n");
    } else {
        TEST_PRINTF("=== TEST FAILED ===\n");
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&barrier_mutex);

    return 0;
}
