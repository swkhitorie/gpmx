#include "./prv_mqueue.h"
#include "utils.h"

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
    int iStatus = 0;
    queuelist_element_t *p = (queuelist_element_t *)mqdes;
    StaticSemaphore_t *queue_listmutex = get_queue_listmutex();
    (void)xSemaphoreTake((SemaphoreHandle_t)queue_listmutex, portMAX_DELAY);

    if (find_queue_inlist(NULL, NULL, mqdes) == pdTRUE) {
        p->attr.mq_curmsgs = (long)uxQueueMessagesWaiting(p->queue);
        *mqstat = p->attr;
    } else {
        // errno = EBADF;
        iStatus = -1;
    }

    (void)xSemaphoreGive((SemaphoreHandle_t)queue_listmutex);
    return iStatus;
}
