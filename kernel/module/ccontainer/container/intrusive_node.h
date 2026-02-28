#ifndef INTRUSIVE_NODE_H_
#define INTRUSIVE_NODE_H_

#include <stdbool.h>
#include "container_of.h"

// intrusive singlylist node
struct intrusive_node {
    struct intrusive_node *next;
};

struct intrusive_queue_head {
    struct intrusive_node *head;
    struct intrusive_node *tail;
};

struct intrusive_list_head {
    struct intrusive_node *head;
};

typedef bool (*intrusive_sorted_list_cmp_method_t)(struct intrusive_node *a, struct intrusive_node *b);

struct intrusive_sorted_list_head {
    struct intrusive_node *head;
    intrusive_sorted_list_cmp_method_t cmpf;
};

#define intrusive_node_init(node)   do { (node)->next = NULL; } while (0)
#define intrusive_queue_init(q)    do { (q)->head = (q)->tail = NULL; } while (0)
#define intrusive_list_init(q)    do { (q)->head = NULL; } while (0)
#define intrusive_sorted_list_init(list, cmp_func) \
    do { (list)->head = NULL; (list)->cmpf = (cmp_func); } while (0)

#ifdef __cplusplus
extern "C" {
#endif

//============================================================  Intrusive Queue
bool intrusive_queue_empty(struct intrusive_queue_head *queue);
struct intrusive_node *intrusive_queue_front(struct intrusive_queue_head *queue);
struct intrusive_node *intrusive_queue_back(struct intrusive_queue_head *queue);
size_t intrusive_queue_size(struct intrusive_queue_head *queue);
// node should init with MACRO intrusive_node_init
void intrusive_queue_push(struct intrusive_queue_head *queue, struct intrusive_node *new_node);
struct intrusive_node *intrusive_queue_pop(struct intrusive_queue_head *queue);
bool intrusive_queue_remove(struct intrusive_queue_head *queue, struct intrusive_node *removeNode);

//============================================================  Intrusive List
bool intrusive_list_empty(struct intrusive_list_head *list);
size_t intrusive_list_size(struct intrusive_list_head *list);
void intrusive_list_delete(struct intrusive_list_head *list, struct intrusive_node *node, void (*free_cb)(struct intrusive_node *));
void intrusive_list_clear(struct intrusive_list_head *list, void (*free_cb)(struct intrusive_node *));
// please confirm new_node is a single node
void intrusive_list_add(struct intrusive_list_head *list, struct intrusive_node *new_node);
bool intrusive_list_remove(struct intrusive_list_head *list, struct intrusive_node *remove_node);

//============================================================  Intrusive Sorted List
bool intrusive_sorted_list_empty(struct intrusive_sorted_list_head *list);
size_t intrusive_sorted_list_size(struct intrusive_sorted_list_head *list);
void intrusive_sorted_list_delete(struct intrusive_sorted_list_head *list, struct intrusive_node *node, void (*free_cb)(struct intrusive_node *));
void intrusive_sorted_list_clear(struct intrusive_sorted_list_head *list, void (*free_cb)(struct intrusive_node *));
// please confirm new_node is a single node
void intrusive_sorted_list_add(struct intrusive_sorted_list_head *list, struct intrusive_node *new_node);
bool intrusive_sorted_list_remove(struct intrusive_sorted_list_head *list, struct intrusive_node *remove_node);

#ifdef __cplusplus
}
#endif

#endif

