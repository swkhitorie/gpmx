#include "sdqueue.h"

/****************************************************************************
 * Name: dq_addfirst
 *
 * Description:
 *  dq_addfirst affs 'node' at the beginning of 'queue'
 *
 ****************************************************************************/

void dq_addfirst(FAR dq_entry_t *node, dq_queue_t *queue)
{
    node->blink = NULL;
    node->flink = queue->head;

    if (!queue->head) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->head->blink = node;
        queue->head = node;
    }
}

