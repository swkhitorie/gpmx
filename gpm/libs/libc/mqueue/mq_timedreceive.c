#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <gpmx/config.h>
#include "./prv_mqueue.h"

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio, const struct timespec *abstime)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    ssize_t ret = 0;
    int cal_timeout_return = 0;
    TickType_t timeout_ticks = 0;
    queuelist_element_t *p = (queuelist_element_t *)mqdes;
    queue_element_t rcv = {0};

    (void)msg_prio;
    queuelist_lock();

    if (find_queue_inlist(NULL, NULL, mqdes) != 0) {
        errno = EBADF;
        ret = -1;
    }

    if (ret == 0) {
        if (msg_len < (size_t)p->attr.mq_msgsize) {
            /* msg_len too small. */
            errno = EMSGSIZE;
            ret = -1;
        }
    }

    if (ret == 0) {
        cal_timeout_return = cal_ticktimeout(p->attr.mq_flags, abstime, &timeout_ticks);
        if (cal_timeout_return != 0) {
            errno = cal_timeout_return;
            ret = -1;
        }
    }

    queuelist_unlock();

    if (ret == 0) {
        if (xQueueReceive(p->queue, &rcv, timeout_ticks) == pdFALSE) {
            /* If queue receive fails, set the appropriate errno. */
            if (p->attr.mq_flags & O_NONBLOCK) {
                /* Set errno to EAGAIN for nonblocking mq. */
                errno = EAGAIN;
            } else {
                /* Otherwise, set errno to ETIMEDOUT. */
                errno = ETIMEDOUT;
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
#else

    return -1;
#endif
}
