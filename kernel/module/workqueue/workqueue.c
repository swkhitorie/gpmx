#include "workqueue.h"
#include "workitem.h"
#include <string.h>
#include "FreeRTOS.h"
#include "kmodule_defines.h"

bool workqueue_cmp_method(struct intrusive_node *a, struct intrusive_node *b)
{
    struct __workqueue *wq_a;
    struct __workqueue *wq_b;
    wq_a = container_of(a, struct __workqueue, node);
    wq_b = container_of(b, struct __workqueue, node);

    return wq_a->_config.relative_priority >= wq_b->_config.relative_priority;
}

/****************************************************************************
 * PRIVATE
 ****************************************************************************/
static void work_lock(struct __workqueue *queue)
{
    queue->_flags = portSET_INTERRUPT_MASK_FROM_ISR();
}

static void work_unlock(struct __workqueue *queue)
{
    portCLEAR_INTERRUPT_MASK_FROM_ISR(queue->_flags);
}

bool should_exit(struct __workqueue *queue)
{ 
    return atomic_bool_load(&queue->_should_exit);
}

static void signal_worker_thread(struct __workqueue *queue)
{
	int sem_val;

	if (sem_getvalue(&queue->_process_lock, &sem_val) == 0 && sem_val <= 0) {
		sem_post(&queue->_process_lock);
	}
}

/****************************************************************************
 * PUBLIC
 ****************************************************************************/
void workqueue_init(struct __workqueue *queue, const struct wq_config_t *config)
{
    queue->_config = *config;

	// set the threads name
	pthread_setname_np(pthread_self(), queue->_config.name);

	sem_init(&queue->_process_lock, 0, 0);
	// sem_setprotocol(&queue->_process_lock, SEM_PRIO_NONE);

	sem_init(&queue->_exit_lock, 0, 1);
	// sem_setprotocol(&queue->_exit_lock, SEM_PRIO_NONE);

    atomic_bool_store(&queue->_should_exit, false);

    intrusive_queue_init(&queue->_q);
    blocking_list_init(&queue->_work_items, workitem_cmp_method);
}

void workqueue_deinit(struct __workqueue *queue)
{
	work_lock(queue);

	// Synchronize with ::Detach
	sem_wait(&queue->_exit_lock);
	sem_destroy(&queue->_exit_lock);
	sem_destroy(&queue->_process_lock);

	work_unlock(queue);
}

const struct wq_config_t *workqueue_getconfig(struct __workqueue *queue)
{
    return &queue->_config;
}

const char *workqueue_getname(struct __workqueue *queue)
{
    return queue->_config.name;
}

bool workqueue_attach(struct __workqueue *queue, struct __workitem *item)
{
    work_lock(queue);

    if (!should_exit(queue)) {
        blocking_list_add(&queue->_work_items, &item->sorted_list_node);
        work_unlock(queue);
        return true;
    }

    work_unlock(queue);
    return false;
}

void workqueue_detach(struct __workqueue *queue, struct __workitem *item)
{
	work_lock(queue);

    blocking_list_remove(&queue->_work_items, &item->sorted_list_node);

	if (blocking_list_size(&queue->_work_items) == 0) {
		// shutdown, no active WorkItems
		KMDEBUG("stopping: %s, last active WorkItem closing", queue->_config.name);

		workqueue_request_stop(queue);
		signal_worker_thread(queue);
	}

	work_unlock(queue);
}

void workqueue_add(struct __workqueue *queue, struct __workitem *item)
{
	work_lock(queue);
    intrusive_queue_push(&queue->_q, &item->queue_node);
	work_unlock(queue);

	signal_worker_thread(queue);
}

void workqueue_remove(struct __workqueue *queue, struct __workitem *item)
{
	work_lock(queue);
    intrusive_queue_remove(&queue->_q, &item->queue_node);
	work_unlock(queue);
}

void workqueue_clear(struct __workqueue *queue)
{
    work_lock(queue);

    while (!intrusive_queue_empty(&queue->_q)) {
        intrusive_queue_pop(&queue->_q);
    }

    work_unlock(queue);
}

void workqueue_run(struct __workqueue *queue)
{
    struct __workitem *work;

    while (!should_exit(queue)) {
        // loop as the wait may be interrupted by a signal
        do {} while (sem_wait(&queue->_process_lock) != 0);

        work_lock(queue);

        // process queued work
        while (!intrusive_queue_empty(&queue->_q)) {
            struct intrusive_node *work_item_node = intrusive_queue_pop(&queue->_q);
            work = container_of(work_item_node, struct __workitem, queue_node);

            work_unlock(queue); // unlock work queue to run (item may requeue itself)

            workitem_run_preamble(work);
            work->entry(NULL);

            // Note: after Run() we cannot access work anymore, as it might have been deleted
            work_lock(queue); // re-lock
        }

        work_unlock(queue);
    }

    KMDEBUG("%s: exiting\r\n", queue->_config.name);
}

void workqueue_request_stop(struct __workqueue *queue)
{
    atomic_bool_store(&queue->_should_exit, true);
}

void workqueue_print_status(struct __workqueue *queue, bool last)
{
    struct __workitem *item;
    struct intrusive_node *iterator = NULL;

    const size_t num_items = blocking_list_size(&queue->_work_items);

    KMINFO("%-16s\n", workqueue_getname(queue));
    unsigned i = 0;

    for (iterator = queue->_work_items.list.head; iterator != NULL; iterator = iterator->next) {
        item = container_of(iterator, struct __workitem, sorted_list_node);

        i++;

        if (last) {
            KMINFO("    ");

        } else {
            KMINFO("|   ");
        }

        if (i < num_items) {
            KMINFO("|__%2d) ", i);

        } else {
            KMINFO("\\__%2d) ", i);
        }

        workitem_printstatus(item);
    }

}


