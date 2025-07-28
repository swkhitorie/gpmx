#include "sdqueue.h"

/****************************************************************************
 * Name: sq_remafter
 *
 * Description:
 *   sq_remafter removes the entry following 'node' from the'queue'  Returns
 *   a reference to the removed entry.
 *
 ****************************************************************************/

FAR sq_entry_t *sq_remafter(FAR sq_entry_t *node, sq_queue_t *queue)
{
    FAR sq_entry_t *ret = node->flink;

    if (queue->head && ret) {
        if (queue->tail == ret) {
            queue->tail = node;
            node->flink = NULL;
        } else {
            node->flink = ret->flink;
        }
        ret->flink = NULL;
    }
    return ret;
}
