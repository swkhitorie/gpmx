#include "sdqueue.h"

/****************************************************************************
 * Name: sq_remfirst
 *
 * Description:
 *   sq_remfirst function removes the first entry from 'queue'
 *
 ****************************************************************************/

FAR sq_entry_t *sq_remfirst(sq_queue_t *queue)
{
    FAR sq_entry_t *ret = queue->head;

    if (ret) {
      queue->head = ret->flink;
      if (!queue->head) {
          queue->tail = NULL;
      }
      ret->flink = NULL;
    }
    return ret;
}

