#include "sdqueue.h"

/****************************************************************************
 * Name: dq_remfirst
 *
 * Description:
 *   dq_remfirst removes 'node' from the head of 'queue'
 *
 ****************************************************************************/

FAR dq_entry_t *dq_remfirst(dq_queue_t *queue)
{
    FAR dq_entry_t *ret = queue->head;

    if (ret) {
        FAR dq_entry_t *next = ret->flink;
        if (!next) {
            queue->head = NULL;
            queue->tail = NULL;
        } else {
            queue->head = next;
            next->blink = NULL;
        }
        ret->flink = NULL;
        ret->blink = NULL;
    }
    return ret;
}
