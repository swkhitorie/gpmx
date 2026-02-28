#include "workitem.h"
#include <string.h>
#include <float.h>
#include <math.h>
#include "kmodule_defines.h"

bool workitem_cmp_method(struct intrusive_node *a, struct intrusive_node *b)
{
    struct __workitem *work_a;
    struct __workitem *work_b;
    work_a = container_of(a, struct __workitem, sorted_list_node);
    work_b = container_of(b, struct __workitem, sorted_list_node);

    return (strcmp(workitem_name(work_a), workitem_name(work_b)) <= 0);
}

void workitem_config_entry(struct __workitem *item, workitem_entry work_main)
{
    item->entry = work_main;
}

bool workitem_init(struct __workitem *item, const char *name, const struct wq_config_t *config)
{
	// clear any existing first
	workitem_deinit(item);

    item->_name = name;
    item->_time_first_run = 0;
    item->_run_count = 0;

    intrusive_node_init(&item->sorted_list_node);
    intrusive_node_init(&item->queue_node);

    struct __workqueue *wq = workqueue_find_or_create(config);

	if ((wq != NULL) && workqueue_attach(wq, item)) {
		item->_wq = wq;
		item->_time_first_run = 0;
		return true;
	}

	KMERROR("%s not available, init failed\r\n", config->name);
	return false;
}

void workitem_init_method2(struct __workitem *item, const char *name, const struct __workitem *work_item)
{
	struct __workqueue *wq = work_item->_wq;

    item->_name = name;

	if ((wq != NULL) && workqueue_attach(wq, item)) {
		item->_wq = wq;
	}
}

bool workitem_change_queue(struct __workitem *item, const struct wq_config_t *config)
{
    return workitem_init(item, item->_name, config);
}

void workitem_deinit(struct __workitem *item)
{
	// remove any currently queued work
	if (item->_wq != NULL) {
		// prevent additional insertions
        struct __workqueue *wq_temp = item->_wq;
        item->_wq = NULL;

		// remove any queued work
        workqueue_remove(wq_temp, item);

        workqueue_detach(wq_temp, item);
	}
}

const char *workitem_name(struct __workitem *item)
{
    return item->_name;
}

void workitem_schedule_now(struct __workitem *item)
{
    if (item->_wq != NULL) {
        workqueue_add(item->_wq, item);
    }
}

void workitem_schedule_clear(struct __workitem *item)
{
    if (item->_wq != NULL) {
        workqueue_remove(item->_wq, item);
    }
}

void workitem_run_preamble(struct __workitem *item)
{
    if (item->_run_count == 0) {
        item->_time_first_run = hrt_absolute_time();
        item->_run_count = 1;

    } else {
        item->_run_count++;
    }
}

float workitem_elapsed_time(struct __workitem *item)
{
    return hrt_elapsed_time(&item->_time_first_run) / 1e6f;
}

float workitem_average_rate(struct __workitem *item)
{
    const float rate = item->_run_count / workitem_elapsed_time(item);

    if ((item->_run_count > 1) && isfinite(rate)) {
        return rate;
    }

    return 0.f;
}

float workitem_average_interval(struct __workitem *item)
{
    const float rate = workitem_average_rate(item);
    const float interval = 1e6f / rate;

    if ((rate > FLT_EPSILON) && isfinite(interval)) {
        return roundf(interval);
    }

    return 0.f;
}

void workitem_printstatus(struct __workitem *item)
{
    KMINFO("%-26s %8.1f Hz %12.0f us\n", item->_name, (double)workitem_average_rate(item), (double)workitem_average_interval(item));
}


