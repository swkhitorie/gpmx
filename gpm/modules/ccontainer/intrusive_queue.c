#include "container/intrusive_node.h"

bool intrusive_queue_empty(struct intrusive_queue_head *queue)
{
    return queue->head == NULL;
}

struct intrusive_node *intrusive_queue_front(struct intrusive_queue_head *queue)
{
    return queue->head;
}

struct intrusive_node *intrusive_queue_back(struct intrusive_queue_head *queue)
{
    return queue->tail;
}

size_t intrusive_queue_size(struct intrusive_queue_head *queue)
{
    size_t sz = 0;
    struct intrusive_node *node;

    for (node = intrusive_queue_front(queue); node != NULL; node = node->next) {
        sz++;
    }

    return sz;
}

// node should init with MACRO intrusive_node_init
void intrusive_queue_push(struct intrusive_queue_head *queue, struct intrusive_node *new_node)
{
    // error, node already queued or already inserted
    if ((new_node->next != NULL) || (new_node == queue->tail)) {
        return;
    }

    if (queue->head == NULL) {
        queue->head = new_node;
    }

    if (queue->tail != NULL) {
        queue->tail->next = new_node;
    }

    queue->tail = new_node;
}

struct intrusive_node *intrusive_queue_pop(struct intrusive_queue_head *queue)
{
    struct intrusive_node *ret = queue->head;

    if (!intrusive_queue_empty(queue)) {
        if (queue->head != queue->tail) {
            queue->head = queue->head->next;

        } else {
            // only one item left
            queue->head = NULL;
            queue->tail = NULL;
        }

        // clear next in popped (in might be re-inserted later)
        ret->next = NULL;
    }

    return ret;
}

bool intrusive_queue_remove(struct intrusive_queue_head *queue, struct intrusive_node *removeNode)
{
    struct intrusive_node *node;
    // base case
    if (removeNode == queue->head) {
        if (queue->head->next != NULL) {
            queue->head = queue->head->next;
            removeNode->next = NULL;

        } else {
            queue->head = NULL;
            queue->tail = NULL;
        }

        return true;
    }

    for (node = queue->head; node != NULL; node = node->next) {
        // is sibling the node to remove?
        if (node->next == removeNode) {
            if (removeNode == queue->tail) {
                queue->tail = node;
            }

            // replace sibling
            node->next = removeNode->next;
            removeNode->next = NULL;
            return true;
        }
    }

    return false;
}
