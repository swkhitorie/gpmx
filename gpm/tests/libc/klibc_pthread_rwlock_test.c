#include <gpmx/config.h>

#include <mlog.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int shared_data = 0;
static pthread_rwlock_t rwlock;

static pthread_barrier_t start_barrier;

void* reader(void* arg)
{
    int id = *(int*)arg;
    int i;

    pthread_barrier_wait(&start_barrier);

    for (i = 0; i < 3; i++) {
        pthread_rwlock_rdlock(&rwlock);
        KMRAW("Reader %d: acquired read lock, shared_data = %d\n", id, shared_data);
        usleep(100000);
        pthread_rwlock_unlock(&rwlock);
        usleep(10000);
    }
    return NULL;
}

void* writer(void* arg)
{
    int id = *(int*)arg;
    int i;

    pthread_barrier_wait(&start_barrier);

    for (i = 0; i < 2; i++) {
        pthread_rwlock_wrlock(&rwlock);
        shared_data++;
        KMRAW("Writer %d: acquired write lock, incremented to %d\n", id, shared_data);
        usleep(200000);
        pthread_rwlock_unlock(&rwlock);
        usleep(20000);
    }
    return NULL;
}

int klibc_pthread_rwlock_test(int argc, char **argv)
{
#define NUM_READERS 3
#define NUM_WRITERS 1
#define TOTAL_THREADS (NUM_READERS + NUM_WRITERS)

    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int reader_ids[NUM_READERS], writer_ids[NUM_WRITERS];
    int i, rc;

    pthread_rwlock_init(&rwlock, NULL);
    pthread_barrier_init(&start_barrier, NULL, TOTAL_THREADS);

    for (i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        rc = pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
        if (rc != 0) {
            perror("pthread_create reader");
            return -2;
        }
    }

    for (i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        rc = pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
        if (rc != 0) {
            perror("pthread_create writer");
            return -1;
        }
    }

    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    pthread_barrier_destroy(&start_barrier);

    KMRAW("All threads finished, final shared_data = %d\n", shared_data);
    return 0;
}
