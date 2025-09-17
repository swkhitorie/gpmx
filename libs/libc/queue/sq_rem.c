#include "sdqueue.h"

/****************************************************************************
 * Name: sq_rem
 *
 * Description:
 *   sq_rem removes a 'node' for 'queue.'
 *
 ****************************************************************************/

void sq_rem(sq_entry_t *node, sq_queue_t *queue)
{
    if (queue->head && node) {
        if (node == queue->head) {
            queue->head = node->flink;
            if (node == queue->tail) {
                queue->tail = NULL;
            }
        } else {
            sq_entry_t *prev;
            for (prev = (sq_entry_t *)queue->head;
                  prev && prev->flink != node;
                  prev = prev->flink);
            if (prev) {
              sq_remafter(prev, queue);
            }
        }
    }
}
