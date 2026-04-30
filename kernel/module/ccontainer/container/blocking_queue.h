#ifndef BLOCKING_QUEUE_H_
#define BLOCKING_QUEUE_H_

#include <semaphore.h>
#include <stdint.h>

#define DEFINE_BLOCKING_QUEUE(T, N, QUEUE_TYPE)                             \
    typedef struct {                                                        \
        sem_t sem_head;                                                     \
        sem_t sem_tail;                                                     \
        size_t head;                                                        \
        size_t tail;                                                        \
        T data[N];                                                          \
    } QUEUE_TYPE;                                                           \
                                                                            \
    static inline void QUEUE_TYPE##_init(QUEUE_TYPE *q) {               \
        sem_init(&q->sem_head, 0, N);                                   \
        sem_init(&q->sem_tail, 0, 0);                                   \
        sem_setprotocol(&q->sem_head, SEM_PRIO_NONE);                   \
        sem_setprotocol(&q->sem_tail, SEM_PRIO_NONE);                   \
        q->head = 0;                                                    \
        q->tail = 0;                                                    \
        uint8_t *p=(uint8_t *)&q->data[0],i=0;                          \
        while(i<sizeof(T)*N) {p[i]=0; i++;}                             \
    }                                                                   \
                                                                        \
    static inline void QUEUE_TYPE##_destroy(QUEUE_TYPE *q) {            \
        sem_destroy(&q->sem_head);                                      \
        sem_destroy(&q->sem_tail);                                      \
    }                                                                   \
                                                                        \
    static inline void QUEUE_TYPE##_push(QUEUE_TYPE *q, T item) {       \
        while (sem_wait(&q->sem_head) != 0);                            \
        q->data[q->tail] = item;                                        \
        q->tail = (q->tail + 1) % N;                                    \
        sem_post(&q->sem_tail);                                         \
    }                                                                   \
                                                                        \
    static inline T QUEUE_TYPE##_pop(QUEUE_TYPE *q) {                        \
        while (sem_wait(&q->sem_tail) != 0);                                 \
        T ret = q->data[q->head];                                            \
        q->head = (q->head + 1) % N;                                         \
        sem_post(&q->sem_head);                                              \
        return ret;                                                          \
    }

/*
    use sample:

    DEFINE_BLOCKING_QUEUE(WorkItem*, 16, work_item_queue_t)

    work_item_queue_t my_queue;
    work_item_queue_init(&my_queue);

    WorkItem *item = create_work_item();
    work_item_queue_push(&my_queue, item);

    WorkItem *processed = work_item_queue_pop(&my_queue);
    work_item_queue_destroy(&my_queue);
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif

