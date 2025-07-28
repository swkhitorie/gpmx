#include "sdqueue.h"

/****************************************************************************
 * Name: dq_rem
 *
 * Description:
 *   dq_rem removes 'node' from 'queue'
 *
 ****************************************************************************/

void dq_rem(FAR dq_entry_t *node, dq_queue_t *queue)
{
    FAR dq_entry_t *prev = node->blink;
    FAR dq_entry_t *next = node->flink;

    if (!prev) {
        queue->head = next;
    } else {
        prev->flink = next;
    }

    if (!next) {
        queue->tail = prev;
    } else {
        next->blink = prev;
    }

    node->flink = NULL;
    node->blink = NULL;
}
