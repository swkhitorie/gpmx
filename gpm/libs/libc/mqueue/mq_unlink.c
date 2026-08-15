#include <mqueue.h>
#include <errno.h>
#include <gpmx/config.h>
#include "./prv_mqueue.h"

int mq_unlink(const char *name)
{
#if defined(CONFIG_RTTNANO_ENABLE)

    return -1;
#elif defined(CONFIG_FREERTOS_ENABLE)

    int ret = 0;
    size_t name_size = 0;
    BaseType_t queue_removed = pdFALSE;
    queuelist_element_t *p = NULL;

    init_queuelist();

    if (validate_queuename(name, &name_size) != 0) {
        errno = EINVAL;
        ret = -1;
    }

    if (ret == 0) {
        queuelist_lock();

        if (find_queue_inlist(&p, name, (mqd_t)NULL) == 0) {
            if (p->open_descriptors == 0) {
                listREMOVE(&p->link);
                queue_removed = pdTRUE;
            } else {
                p->pending_unlink = pdTRUE;
            }
        } else {
            errno = ENOENT;
            ret = -1;
        }

        queuelist_unlock();
    }

    if (queue_removed == pdTRUE) {
        delete_messagequeue(p);
    }
    return ret;
#else

    return -1;
#endif
}
