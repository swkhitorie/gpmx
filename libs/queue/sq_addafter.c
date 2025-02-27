#include "sdqueue.h"

/****************************************************************************
 * Name: sq_addafter.c
 *
 * Description:
 *  The sq_addafter function adds 'node' after 'prev' in the 'queue.'
 *
 ****************************************************************************/

void sq_addafter(FAR sq_entry_t *prev, FAR sq_entry_t *node, sq_queue_t *queue)
{
    if (!queue->head || prev == queue->tail) {
        sq_addlast(node, queue);
    } else {
        node->flink = prev->flink;
        prev->flink = node;
    }
}
