#ifndef PRV_MQUEUE_H_
#define PRV_MQUEUE_H_

#include <gpmx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <mqueue.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>

#include "./dlist.h"

typedef struct __queuedata {
    char *p;
    size_t size;
} queue_element_t;

typedef struct QueueListElement {
    Link_t link;
    QueueHandle_t queue;
    size_t open_descriptors;
    char * name;
    struct mq_attr attr;
    BaseType_t pending_unlink;
} queuelist_element_t;

#ifdef __cplusplus
extern "C" {
#endif

void queuelist_lock();
void queuelist_unlock();
void init_queuelist();

int find_queue_inlist(queuelist_element_t **p, const char *name, mqd_t descriptor);
int create_new_messagequeue(queuelist_element_t **p,
    const struct mq_attr *pattr, const char *name, size_t len);
void delete_messagequeue(const queuelist_element_t *p);

int cal_ticktimeout(long flag, const struct timespec *ptimeout, TickType_t *ptimeout_ticks);
int validate_queuename(const char *name, size_t *len);

#ifdef __cplusplus
}
#endif

#endif


#endif
