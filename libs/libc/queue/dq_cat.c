#include "sdqueue.h"

/****************************************************************************
 * Name: dq_cat
 *
 * Description:
 *   Move the content of queue1 to the end of queue2.
 *
 ****************************************************************************/

void dq_cat(FAR dq_queue_t *queue1, FAR dq_queue_t *queue2)
{
    if (dq_empty(queue2)) {
        /* If queue2 is empty, then just move queue1 to queue2 */
        dq_move(queue1, queue2);
        /* Do nothing if queue1 is empty */
    } else if (!dq_empty(queue1)) {
        /* Attach the head of queue1 to the final entry of queue2 */
        queue2->tail->flink = queue1->head;
        queue1->head->blink = queue2->tail;
        /* The tail of queue1 is the new tail of queue2 */
        queue2->tail = queue1->tail;
        dq_init(queue1);
    }
}
