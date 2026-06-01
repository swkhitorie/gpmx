#include <mqueue.h>
#include <errno.h>

#include "./prv_mqueue.h"

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    queuelist_element_t *p = (queuelist_element_t *)mqdes;

    queuelist_lock();

    if (find_queue_inlist(NULL, NULL, mqdes) == 0) {
        p->attr.mq_curmsgs = (long)uxQueueMessagesWaiting(p->queue);
        *mqstat = p->attr;
    } else {
        errno = EBADF;
        ret = -1;
    }

    queuelist_unlock();
    return ret;
#else

    return -1;
#endif
}
