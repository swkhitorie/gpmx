#include <gpmx/config.h>

#include <stdint.h>

#include "workitem.h"
#include "scheduledworkitem.h"
#include "workqueue_manager.h"

#include "wqueue_all_test.h"
#include "wqueue_scheduled_test.h"
#include "wqueue_test.h"
#include <unistd.h>

int wqueue_all_test(int argc, char **argv)
{
    hrt_init();
    workqueue_manager_start();

    sleep(1);

    extern void wqueue_test_init();
    wqueue_test_init();

    extern void wqueue_sched_test_init();
    wqueue_sched_test_init();

    extern int wqueue_main();
    wqueue_main();

    extern int wqueue_sched_main();
    wqueue_sched_main();

    workqueue_manager_status();
}

