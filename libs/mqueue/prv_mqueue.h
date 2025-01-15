#ifndef PRV_MQUEUE_H_
#define PRV_MQUEUE_H_

#include <stdint.h>
#include <stdbool.h>
#include "mqueue.h"
#include "errno.h"
#include "fcntl.h"

/**
 * FreeRTOS Include
 */
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>
#include "./dlist.h"

typedef struct __queuedata
{
    char *p;
    size_t size;
} queue_element_t;

typedef struct QueueListElement
{
    Link_t link;
    QueueHandle_t queue;
    size_t open_descriptors;
    char * name;
    struct mq_attr attr;
    BaseType_t pending_unlink;
} queuelist_element_t;

StaticSemaphore_t *get_queue_listmutex();
Link_t *get_queue_listhead();

int cal_ticktimeout(long flag, const struct timespec *ptimeout, TickType_t *ptimeout_ticks);

BaseType_t create_new_messagequeue(queuelist_element_t **p,
    const struct mq_attr *pattr, const char *name, size_t len);

void delete_messagequeue(const queuelist_element_t *p);

BaseType_t find_queue_inlist(queuelist_element_t **p, const char *name, mqd_t descriptor);

void init_queuelist(void);

BaseType_t validate_queuename(const char *name, size_t *len);



#endif
