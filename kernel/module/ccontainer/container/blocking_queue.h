#ifndef BLOCKING_QUEUE_H_
#define BLOCKING_QUEUE_H_

#include <semaphore.h>

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

// 1. 生成队列类型和函数
DEFINE_BLOCKING_QUEUE(WorkItem*, 16, work_item_queue_t)

// 2. 声明并初始化队列
work_item_queue_t my_queue;
work_item_queue_init(&my_queue);

// 3. 在生产者任务中推送元素
WorkItem *item = create_work_item();
work_item_queue_push(&my_queue, item);

// 4. 在消费者任务中弹出元素
WorkItem *processed = work_item_queue_pop(&my_queue);
// 使用 processed...

// 5. 不再使用时销毁（释放信号量资源）
work_item_queue_destroy(&my_queue);

*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif

