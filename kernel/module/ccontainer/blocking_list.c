#include "container/blocking_list.h"

void blocking_list_deinit(struct blocking_list *blklist)
{
    pthread_mutex_destroy(&blklist->_mutex);
}

void blocking_list_add(struct blocking_list *blklist, struct intrusive_node *node)
{
    int b = pthread_mutex_lock(&blklist->_mutex);
    intrusive_sorted_list_add(&blklist->list, node);
    int a = pthread_mutex_unlock(&blklist->_mutex);
}

bool blocking_list_remove(struct blocking_list *blklist, struct intrusive_node *node)
{
    bool ret = false;
    pthread_mutex_lock(&blklist->_mutex);
    ret = intrusive_sorted_list_remove(&blklist->list, node);
    pthread_mutex_unlock(&blklist->_mutex);

    return ret;
}

size_t blocking_list_size(struct blocking_list *blklist)
{
    size_t ret = 0;
    pthread_mutex_lock(&blklist->_mutex);
    ret = intrusive_sorted_list_size(&blklist->list);
    pthread_mutex_unlock(&blklist->_mutex);

    return ret;
}

void blocking_list_clear(struct blocking_list *blklist, void (*free_cb)(struct intrusive_node *))
{
    pthread_mutex_lock(&blklist->_mutex);
    intrusive_sorted_list_clear(&blklist->list, free_cb);
    pthread_mutex_unlock(&blklist->_mutex);
}
