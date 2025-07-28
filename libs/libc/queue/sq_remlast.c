#include "sdqueue.h"

/****************************************************************************
 * Name: sq_remlast
 *
 * Description:
 *   Removes the last entry in a singly-linked queue.
 *
 ****************************************************************************/

FAR sq_entry_t *sq_remlast(sq_queue_t *queue)
{
    FAR sq_entry_t *ret = queue->tail;

    if (ret) {
        if (queue->head == queue->tail) {
            queue->head = NULL;
            queue->tail = NULL;
        } else {
            FAR sq_entry_t *prev;
            for (prev = queue->head;
                prev && prev->flink != ret;
                prev = prev->flink);

            if (prev) {
                prev->flink = NULL;
                queue->tail = prev;
            }
        }
        ret->flink = NULL;
    }
    return ret;
}
