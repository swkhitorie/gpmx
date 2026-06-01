#include <mqueue.h>
#include <errno.h>

#include "./prv_mqueue.h"

int mq_close(mqd_t mqdes)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    queuelist_element_t *p = (queuelist_element_t *)mqdes;
    int removed = -1;
    init_queuelist();

    queuelist_lock();

    /* Attempt to find the message queue based on the given descriptor. */
    if (find_queue_inlist(NULL, NULL, mqdes) == 0) {
        if (p->open_descriptors > 0){
            p->open_descriptors--;
        }

        if (p->open_descriptors == 0) {
            if (p->pending_unlink == pdTRUE) {
                listREMOVE(&p->link);
                removed = 0;
            } else {
                p->pending_unlink = pdTRUE;
            }
        }
    } else {
        errno = EBADF;
        ret = -1;
    }

    queuelist_unlock();

    if (removed == 0) {
        delete_messagequeue(p);
    }
    return ret;
#else

    return -1;
#endif
}
