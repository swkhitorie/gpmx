#include "sdqueue.h"

/****************************************************************************
 * Name: dq_remlast
 *
 * Description:
 *   dq_remlast removes the last entry from 'queue'
 *
 ****************************************************************************/

FAR dq_entry_t *dq_remlast(dq_queue_t *queue)
{
    FAR dq_entry_t *ret = queue->tail;

    if (ret) {
        FAR dq_entry_t *prev = ret->blink;
        if (!prev) {
            queue->head = NULL;
            queue->tail = NULL;
        } else {
            queue->tail = prev;
            prev->flink = NULL;
        }
        ret->flink = NULL;
        ret->blink = NULL;
    }
    return ret;
}

