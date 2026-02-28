#include "wqueue_scheduled_test.h"
#include "scheduledworkitem.h"
#include "workqueue_manager.h"

#include "kmodule_defines.h"
#include <unistd.h>

struct __scheduledworkitem wqueue_sched_test_item;
char *wqueue_sched_witem_name = "wqueue_sched_test";
const struct wq_config_t wqueue_sched_witem_config = {"wq:test2", 2000, -13};
uint32_t wqueue_run_cnt_2 = 0;
bool wqueue_app_state_2 = false;   // false:running, true:request exit

static void *wqueue_sched_run(void *p);

void wqueue_sched_test_init()
{
    workitem_config_entry(&wqueue_sched_test_item.item, wqueue_sched_run);
    workitem_init(&wqueue_sched_test_item.item, wqueue_sched_witem_name, &wqueue_sched_witem_config);
}

void *wqueue_sched_run(void *p)
{
    if (wqueue_run_cnt_2 > 1000) {
        wqueue_app_state_2 = true;
    }

    // KMDEBUG("wsched_item: %llu\r\n", hrt_absolute_time());

    wqueue_run_cnt_2++;
}

int wqueue_sched_main()
{
    wqueue_app_state_2 = false;

    wqueue_run_cnt_2 = 0;

    // Put work in the work queue
    schedule_workitem_oninterval(&wqueue_sched_test_item, 4000, 0);

    // Wait for work to finsh
    while (!wqueue_app_state_2) {
        usleep(10000);
    }

    KMINFO("WQueueScheduledTest finished: %d\r\n", wqueue_run_cnt_2);

    sleep(2);

    return 0;
}

int wqueue_scheduled_test(int argc, char **argv)
{
    hrt_init();
    workqueue_manager_start();

    wqueue_sched_test_init();
    wqueue_sched_main();

    workqueue_manager_status();
    return 0;
}





