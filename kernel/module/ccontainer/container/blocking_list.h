#ifndef BLOCKING_LIST_H_
#define BLOCKING_LIST_H_

#include "intrusive_node.h"
#include <pthread.h>
#include <stdlib.h>

struct blocking_list {
    struct intrusive_sorted_list_head list;
    pthread_mutex_t	_mutex;
};

#define blocking_list_init(blklist, cmp_func) \
    do { intrusive_sorted_list_init(&(blklist)->list, cmp_func); (blklist)->_mutex = PTHREAD_MUTEX_INITIALIZER; } while (0)

#ifdef __cplusplus
extern "C" {
#endif

void blocking_list_deinit(struct blocking_list *blklist);
void blocking_list_add(struct blocking_list *blklist, struct intrusive_node *node);
bool blocking_list_remove(struct blocking_list *blklist, struct intrusive_node *node);
size_t blocking_list_size(struct blocking_list *blklist);
void blocking_list_clear(struct blocking_list *blklist, void (*free_cb)(struct intrusive_node *));

#ifdef __cplusplus
}
#endif

#endif

