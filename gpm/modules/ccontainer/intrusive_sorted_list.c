#include "container/intrusive_node.h"

//============================================================  Intrusive Sorted List
bool intrusive_sorted_list_empty(struct intrusive_sorted_list_head *list)
{
    return list->head == NULL; 
}

size_t intrusive_sorted_list_size(struct intrusive_sorted_list_head *list)
{
    struct intrusive_node *node = list->head;
    size_t sz = 0;

    for (; node != NULL; node = node->next) {
        sz++;
    }

    return sz;
}

void intrusive_sorted_list_delete(struct intrusive_sorted_list_head *list, struct intrusive_node *node, void (*free_cb)(struct intrusive_node *))
{
    if (!free_cb) {
        // do nothing
        return;
    }

    if (intrusive_sorted_list_remove(list, node)) {
        // only delete if node was successfully removed

        free_cb(node);
    }
}

void intrusive_sorted_list_clear(struct intrusive_sorted_list_head *list, void (*free_cb)(struct intrusive_node *))
{
    struct intrusive_node *node = list->head;
    struct intrusive_node *next;

    list->head = NULL;

    if (!free_cb) {
        // do nothing
        return;
    }

    while (node != NULL) {
        next = node->next;
        free_cb(node);
        node = next;
    }
}

// please confirm new_node is a single node
void intrusive_sorted_list_add(struct intrusive_sorted_list_head *list, struct intrusive_node *new_node)
{
    struct intrusive_node *node;
    new_node->next = NULL;

    if (list->head == NULL) {
        // list is empty, add as head
        list->head = new_node;
        return;

    } else {
        if (list->cmpf(new_node, list->head)) {
            new_node->next = list->head;
            list->head = new_node;
            return;
        }

        // find last node and add to end
        node = list->head;

        while (node != NULL && node->next != NULL) {

            if (list->cmpf(new_node, node->next)) {
                // insert newNode
                new_node->next = node->next;
                node->next = new_node;
                return;
            }

            node = node->next;
        }

        // reached the end, add
        node->next = new_node;
    }
}

bool intrusive_sorted_list_remove(struct intrusive_sorted_list_head *list, struct intrusive_node *remove_node)
{
    struct intrusive_node *node;

    if (remove_node == NULL) {
        return false;
    }

    // base case
    if (remove_node == list->head) {
        if (list->head != NULL) {
            list->head = list->head->next;
        }

        remove_node->next = NULL;

        return true;
    }

    for (node = list->head; node != NULL; node = node->next) {
        // is sibling the node to remove?
        if (node->next == remove_node) {
            // replace sibling
            node->next = node->next->next;
            remove_node->next = NULL;

            return true;
        }
    }

    return false;
}

