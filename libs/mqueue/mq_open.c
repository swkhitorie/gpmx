#include "./prv_mqueue.h"
#include "utils.h"

#define posixconfigMQ_MAX_MESSAGES       10 /**< Maximum number of messages in an mq at one time. */
#define posixconfigMQ_MAX_SIZE           128 /**< Maximum size (in bytes) of each message. */

mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr)
{
    mqd_t msg_queue = NULL;
    size_t name_len = 0;
    StaticSemaphore_t *queue_listmutex = get_queue_listmutex();
    /* Default mq_attr. */
    struct mq_attr queue_creation_attr =
    {
        .mq_flags   = 0,
        .mq_maxmsg  = posixconfigMQ_MAX_MESSAGES,
        .mq_msgsize = posixconfigMQ_MAX_SIZE,
        .mq_curmsgs = 0
    };

    (void)mode;
    init_queuelist();
    if (validate_queuename(name, &name_len) == pdFALSE) {
        // errno = EINVAL;
        msg_queue = (mqd_t)-1;
    }

    if (msg_queue == NULL) {
        if ((oflag & O_CREAT) && (attr != NULL) && ((attr->mq_maxmsg <= 0) || (attr->mq_msgsize <= 0))) {
            /* Invalid mq_attr.mq_maxmsg or mq_attr.mq_msgsize. */
            // errno = EINVAL;
            msg_queue = (mqd_t)-1;
        }
    }

    if (msg_queue == NULL) {
        (void)xSemaphoreTake((SemaphoreHandle_t)queue_listmutex, portMAX_DELAY);
        if (find_queue_inlist((queuelist_element_t **)&msg_queue, name, (mqd_t)NULL) == pdTRUE) {
            if ((oflag & O_EXCL) && (oflag & O_CREAT)) {
                // errno = EEXIST;
                msg_queue = (mqd_t)-1;
            } else {
                if(((queuelist_element_t *)msg_queue)->pending_unlink == pdTRUE) {
                    /* Queue pending deletion. Don't allow it to be re-opened. */
                    // errno = EINVAL;
                    msg_queue = ( mqd_t ) -1;
                } else {
                    /* Increase count of open file descriptors for queue. */
                    ((queuelist_element_t *)msg_queue)->open_descriptors++;
                }
            }
        } else {
            /* Only create the new queue if O_CREAT was specified. */
            if (oflag & O_CREAT) {
                /* Copy attributes if provided. */
                if (attr != NULL) {
                    queue_creation_attr = *attr;
                }
                queue_creation_attr.mq_flags = (long)oflag;
                if (create_new_messagequeue((queuelist_element_t **)&msg_queue,
                    &queue_creation_attr, name, name_len) == pdFALSE ) {
                    // errno = ENOSPC;
                    msg_queue = (mqd_t)-1;
                }
            } else {
                // errno = ENOENT;
                msg_queue = (mqd_t)-1;
            }
        }
        (void)xSemaphoreGive((SemaphoreHandle_t)queue_listmutex);
    }
    return msg_queue;
}
