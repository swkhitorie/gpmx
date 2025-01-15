#include "./prv_mqueue.h"
#include "utils.h"

int mq_close(mqd_t mqdes)
{
    int ret = 0;
    queuelist_element_t *p = (queuelist_element_t *)mqdes;
    BaseType_t removed = pdFALSE;
    StaticSemaphore_t *queue_listmutex = get_queue_listmutex();
    init_queuelist();

    (void)xSemaphoreTake((SemaphoreHandle_t)queue_listmutex, portMAX_DELAY);

    /* Attempt to find the message queue based on the given descriptor. */
    if (find_queue_inlist(NULL, NULL, mqdes) == pdTRUE) {
        if (p->open_descriptors > 0){
            p->open_descriptors--;
        }

        if (p->open_descriptors == 0) {
            if (p->pending_unlink == pdTRUE) {
                listREMOVE(&p->link);
                removed = pdTRUE;
            } else {
                p->pending_unlink = pdTRUE;
            }
        }
    } else {
        // errno = EBADF;
        ret = -1;
    }

    (void)xSemaphoreGive((SemaphoreHandle_t)queue_listmutex);

    if (removed == pdTRUE) {
        delete_messagequeue(p);
    }
    return ret;
}
