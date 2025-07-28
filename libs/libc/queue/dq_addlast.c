#include "sdqueue.h"


/****************************************************************************
 * Name: dq_addlast
 *
 * Description:
 *   dq_addlast adds 'node' to the end of 'queue'
 *
 ****************************************************************************/

void dq_addlast(FAR dq_entry_t *node, dq_queue_t *queue)
{
    node->flink = NULL;
    node->blink = queue->tail;

    if (!queue->head) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->flink = node;
        queue->tail        = node;
    }
}
