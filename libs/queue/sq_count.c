#include "sdqueue.h"

/****************************************************************************
 * Name: sq_count
 *
 * Description:
 *   Return the number of nodes in the queue.
 *
 ****************************************************************************/

size_t sq_count(FAR sq_queue_t *queue)
{
    FAR sq_entry_t *node;
    size_t count;

    for (node = queue->head, count = 0;
          node != NULL;
          node = node->flink, count++);

    return count;
}
