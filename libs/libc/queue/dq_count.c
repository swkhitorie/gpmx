#include "sdqueue.h"

/****************************************************************************
 * Name: dq_count
 *
 * Description:
 *   Return the number of nodes in the queue.
 *
 ****************************************************************************/

size_t dq_count(FAR dq_queue_t *queue)
{
    FAR dq_entry_t *node;
    size_t count;

    for (node = queue->head, count = 0;
        node != NULL;
        node = node->flink, count++);

    return count;
}
