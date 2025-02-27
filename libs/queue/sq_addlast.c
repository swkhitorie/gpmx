#include "sdqueue.h"

/****************************************************************************
 * Name: sq_addlast
 *
 * Description:
 *   The sq_addlast function places the 'node' at the tail of
 *   the 'queue'
 ****************************************************************************/

void sq_addlast(FAR sq_entry_t *node, sq_queue_t *queue)
{
    node->flink = NULL;
    if (!queue->head) {
      queue->head = node;
      queue->tail = node;
    } else {
      queue->tail->flink = node;
      queue->tail        = node;
    }
}
