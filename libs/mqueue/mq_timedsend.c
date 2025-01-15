#include "./prv_mqueue.h"
#include "utils.h"

int mq_timedsend( mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *abstime)
{
    int ret = 0, cal_timeout_return = 0;
    TickType_t timeout_ticks = 0;
    queuelist_element_t *p = ( queuelist_element_t * ) mqdes;
    queue_element_t send = { 0 };
    StaticSemaphore_t *queue_listmutex = get_queue_listmutex();

    (void)msg_prio;
    (void)xSemaphoreTake((SemaphoreHandle_t)queue_listmutex, portMAX_DELAY);

    if (find_queue_inlist(NULL, NULL, mqdes) == pdFALSE) {
        // errno = EBADF;
        ret = -1;
    }

    if (ret == 0) {
        if (msg_len > (size_t)p->attr.mq_msgsize) {
            /* msg_len too large. */
            // errno = EMSGSIZE;
            ret = -1;
        }
    }

    if (ret == 0) {
        cal_timeout_return = cal_ticktimeout( p->attr.mq_flags, abstime, &timeout_ticks);
        if (cal_timeout_return != 0) {
            // errno = cal_timeout_return;
            ret = -1;
        }
    }

    (void)xSemaphoreGive((SemaphoreHandle_t)queue_listmutex);

    if (ret == 0) {
        send.size = msg_len;
        send.p = pvPortMalloc(msg_len);
        if (send.p == NULL) {
            // errno = EMSGSIZE;
            ret = -1;
        } else {
            (void)memcpy(send.p, msg_ptr, msg_len);
        }
    }

    if (ret == 0) {
        if (xQueueSend(p->queue, &send, timeout_ticks ) == pdFALSE ) {
            if (p->attr.mq_flags & O_NONBLOCK ) {
                /* Set errno to EAGAIN for nonblocking mq. */
                // errno = EAGAIN;
            } else {
                /* Otherwise, set errno to ETIMEDOUT. */
                // errno = ETIMEDOUT;
            }
            vPortFree(send.p);
            ret = -1;
        }
    }
    return ret;
}

