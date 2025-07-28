#include "sdqueue.h"

/****************************************************************************
 * Name: sq_cat
 *
 * Description:
 *   Move the content of queue1 to the end of queue2.
 *
 ****************************************************************************/

void sq_cat(FAR sq_queue_t *queue1, FAR sq_queue_t *queue2)
{
    if (sq_empty(queue2)) {
        /* If queue2 is empty, then just move queue1 to queue2 */
        sq_move(queue1, queue2);
        /* Do nothing if queue1 is empty */
    } else if (!sq_empty(queue1)) {
        /* Attach the head of queue1 to the final entry of queue2 */
        queue2->tail->flink = queue1->head;
        /* The tail of queue1 is the new tail of queue2 */
        queue2->tail = queue1->tail;
        sq_init(queue1);
    }
}
