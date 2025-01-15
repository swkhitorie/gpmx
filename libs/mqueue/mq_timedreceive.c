#include "./prv_mqueue.h"
#include "utils.h"

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio, const struct timespec *abstime)
{
    ssize_t ret = 0;
    int cal_timeout_return = 0;
    TickType_t timeout_ticks = 0;
    queuelist_element_t *p = (queuelist_element_t *)mqdes;
    queue_element_t rcv = { 0 };
    StaticSemaphore_t *queue_listmutex = get_queue_listmutex();
    (void)msg_prio;
    (void)xSemaphoreTake((SemaphoreHandle_t)queue_listmutex, portMAX_DELAY);

    if (find_queue_inlist(NULL, NULL, mqdes) == pdFALSE) {
        // errno = EBADF;
        ret = -1;
    }

    if (ret == 0) {
        if (msg_len < (size_t)p->attr.mq_msgsize) {
            /* msg_len too small. */
            // errno = EMSGSIZE;
            ret = -1;
        }
    }

    if (ret == 0) {
        cal_timeout_return = cal_ticktimeout(p->attr.mq_flags, abstime, &timeout_ticks);
        if( cal_timeout_return != 0 ) {
            // errno = cal_timeout_return;
            ret = -1;
        }
    }

    (void)xSemaphoreGive((SemaphoreHandle_t)queue_listmutex);

    if (ret == 0) {
        if (xQueueReceive(p->queue, &rcv, timeout_ticks) == pdFALSE) {
            /* If queue receive fails, set the appropriate errno. */
            if( p->attr.mq_flags & O_NONBLOCK ) {
                /* Set errno to EAGAIN for nonblocking mq. */
                // errno = EAGAIN;
            } else {
                /* Otherwise, set errno to ETIMEDOUT. */
                // errno = ETIMEDOUT;
            }
            ret = -1;
        }
    }

    if (ret == 0) {
        ret = (ssize_t)rcv.size;
        (void)memcpy(msg_ptr, rcv.p, rcv.size);
        vPortFree(rcv.p);
    }

    return ret;
}
