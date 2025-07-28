#include "sdqueue.h"

/****************************************************************************
 * Name: dq_addafter
 *
 * Description:
 *  dq_addafter function adds 'node' after 'prev' in the 'queue.'
 *
 ****************************************************************************/
void dq_addafter(FAR dq_entry_t *prev, FAR dq_entry_t *node, dq_queue_t *queue)
{
    if (!queue->head || prev == queue->tail) {
        dq_addlast(node, queue);
    } else {
        FAR dq_entry_t *next = prev->flink;
        node->blink = prev;
        node->flink = next;
        next->blink = node;
        prev->flink = node;
    }
}
