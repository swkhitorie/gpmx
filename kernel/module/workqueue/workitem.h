#ifndef WORK_ITEM_H_
#define WORK_ITEM_H_

#include <stdint.h>
#include "container/intrusive_node.h"

#include "workqueue_manager.h"
#include "workqueue.h"
#include "drv_hrt.h"

typedef void *(*workitem_entry)(void *);

struct __workitem {
    struct intrusive_node sorted_list_node;
    struct intrusive_node queue_node;

    workitem_entry entry;

    hrt_abstime	_time_first_run;
    const char 	*_name;
    uint32_t	_run_count;

    struct __workqueue *_wq;
};

#ifdef __cplusplus
extern "C" {
#endif

void workitem_config_entry(struct __workitem *item, workitem_entry work_main);
bool workitem_init(struct __workitem *item, const char *name, const struct wq_config_t *config);
void workitem_init_method2(struct __workitem *item, const char *name, const struct __workitem *work_item);
bool workitem_change_queue(struct __workitem *item, const struct wq_config_t *config);
void workitem_deinit(struct __workitem *item);

const char *workitem_name(struct __workitem *item);

void workitem_schedule_now(struct __workitem *item);
void workitem_schedule_clear(struct __workitem *item);

void workitem_run_preamble(struct __workitem *item);
float workitem_elapsed_time(struct __workitem *item);
float workitem_average_rate(struct __workitem *item);
float workitem_average_interval(struct __workitem *item);

void workitem_printstatus(struct __workitem *item);

bool workitem_cmp_method(struct intrusive_node *a, struct intrusive_node *b);

#ifdef __cplusplus
}
#endif

#endif
