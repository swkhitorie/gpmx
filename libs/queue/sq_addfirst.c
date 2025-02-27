#include "sdqueue.h"

/****************************************************************************
 * Name: sq_addfirst
 *
 * Description:
 *   The sq_addfirst function places the 'node' at the head of the 'queue'
 *
 ****************************************************************************/

void sq_addfirst(FAR sq_entry_t *node, sq_queue_t *queue)
{
    node->flink = queue->head;
    if (!queue->head) {
      queue->tail = node;
    }

    queue->head = node;
}
