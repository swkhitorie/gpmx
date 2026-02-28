#include "wqueue_test.h"
#include "workitem.h"
#include "workqueue_manager.h"

#include "kmodule_defines.h"
#include <unistd.h>

struct __workitem wqueue_test_witem;
char *wqueue_witem_name = "wqueue_test";
const struct wq_config_t wqueue_witem_config = {"wq:test1", 2000, -14};
uint32_t wqueue_run_cnt = 0;
bool wqueue_app_state = false;   // false:running, true:request exit

static void *wqueue_run(void *p);

void wqueue_test_init()
{
    workitem_config_entry(&wqueue_test_witem, wqueue_run);
    workitem_init(&wqueue_test_witem, wqueue_witem_name, &wqueue_witem_config);
}

void *wqueue_run(void *p)
{
    if (wqueue_run_cnt > 200000) {
        wqueue_app_state = true;

    } else {
        workitem_schedule_now(&wqueue_test_witem);
    }

    wqueue_run_cnt++;
}

int wqueue_main()
{
    wqueue_app_state = false;

    wqueue_run_cnt = 0;

    // Put work in the work queue
    workitem_schedule_now(&wqueue_test_witem);

    // Wait for work to finsh
    while (!wqueue_app_state) {
        usleep(5000);
    }

    KMINFO("WQueueTest finished: %d\r\n", wqueue_run_cnt);

    //print_status();

    sleep(2);

    return 0;
}

int wqueue_test(int argc, char **argv)
{
    workqueue_manager_start();

    wqueue_test_init();
    wqueue_main();

    workqueue_manager_status();
    return 0;
}
