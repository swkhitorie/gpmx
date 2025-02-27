#include "sdqueue.h"

/****************************************************************************
 * Name: dq_addbefore
 *
 * Description:
 *   dq_addbefore adds 'node' before 'next' in 'queue'
 *
 ****************************************************************************/

void dq_addbefore(FAR dq_entry_t *next, FAR dq_entry_t *node, dq_queue_t *queue)
{
    if (!queue->head || next == queue->head) {
        dq_addfirst(node, queue);
    } else {
        FAR dq_entry_t *prev = next->blink;
        node->flink = next;
        node->blink = prev;
        prev->flink = node;
        next->blink = node;
    }
}
