#include "wqueue_all_test.h"
#include "workitem.h"
#include "scheduledworkitem.h"
#include "workqueue_manager.h"

#include "wqueue_scheduled_test.h"
#include "wqueue_test.h"

int wqueue_all_test(int argc, char **argv)
{
    hrt_init();
    workqueue_manager_start();

    wqueue_test_init();
    wqueue_sched_test_init();
    wqueue_main();
    wqueue_sched_main();

    workqueue_manager_status();

}

