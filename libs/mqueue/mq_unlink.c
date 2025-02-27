#include "./prv_mqueue.h"
#include "utils.h"

int mq_unlink(const char *name)
{
    int ret = 0;
    size_t name_size = 0;
    BaseType_t queue_removed = pdFALSE;
    queuelist_element_t *p = NULL;
    StaticSemaphore_t *queue_listmutex = get_queue_listmutex();

    init_queuelist();

    if (validate_queuename(name, &name_size) == pdFALSE) {
        // errno = EINVAL;
        ret = -1;
    }

    if (ret == 0) {
        (void)xSemaphoreTake((SemaphoreHandle_t)queue_listmutex, portMAX_DELAY);
        if (find_queue_inlist(&p, name, (mqd_t)NULL) == pdTRUE) {
            if (p->open_descriptors == 0) {
                listREMOVE(&p->link);
                queue_removed = pdTRUE;
            } else {
                p->pending_unlink = pdTRUE;
            }
        } else {
            // errno = ENOENT;
            ret = -1;
        }
        (void)xSemaphoreGive((SemaphoreHandle_t)queue_listmutex);
    }

    if( queue_removed == pdTRUE ) {
        delete_messagequeue(p);
    }
    return ret;
}
