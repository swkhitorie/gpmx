#ifndef WORK_QUEUE_H_
#define WORK_QUEUE_H_

#include <stdint.h>
#include "container/atomic.h"
#include "container/intrusive_node.h"
#include "container/blocking_list.h"
#include "workqueue_manager.h"

#include <semaphore.h>

struct __workitem;

struct __workqueue {
    struct intrusive_node node;

    uint32_t _flags;
    //IntrusiveQueue<WorkItem *>	_q;
    //BlockingList<WorkItem *>	_work_items;
    struct intrusive_queue_head _q;
    sem_t     _process_lock;
    sem_t     _exit_lock;
    struct wq_config_t _config;
    struct blocking_list _work_items;
	atomic_bool_t _should_exit;
};

#ifdef __cplusplus
extern "C" {
#endif

void workqueue_init(struct __workqueue *queue, const struct wq_config_t *config);
void workqueue_deinit(struct __workqueue *queue);

const struct wq_config_t *workqueue_getconfig(struct __workqueue *queue);
const char *workqueue_getname(struct __workqueue *queue);

bool workqueue_attach(struct __workqueue *queue, struct __workitem *item);
void workqueue_detach(struct __workqueue *queue, struct __workitem *item);
void workqueue_add(struct __workqueue *queue, struct __workitem *item);
void workqueue_remove(struct __workqueue *queue, struct __workitem *item);
void workqueue_clear(struct __workqueue *queue);
void workqueue_run(struct __workqueue *queue);
void workqueue_request_stop(struct __workqueue *queue);
void workqueue_print_status(struct __workqueue *queue, bool last);

bool workqueue_cmp_method(struct intrusive_node *a, struct intrusive_node *b);

#ifdef __cplusplus
}
#endif

#endif
