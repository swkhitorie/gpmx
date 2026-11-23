#include <mlog.h>
#include "workqueue_manager.h"
#include <string.h>

#if !defined(CONFIG_MODULE_NSHLIB)
#if defined(CONFIG_GMSH)
#include "gmsh.h"
#endif

#if defined(CONFIG_FINSH)
#include "shell.h"
#endif
#endif

int workqueue_main(int argc, char **argv)
{
    if (argc != 2) {
        KMINFO("Usage: work_queue status/start/stop\r\n");
        return 1;
    }

    if (!strcmp(argv[1], "start")) {
        workqueue_manager_start();
        return 0;

    } else if (!strcmp(argv[1], "stop")) {
        workqueue_manager_stop();
        return 0;

    } else if (!strcmp(argv[1], "status")) {
        workqueue_manager_status();
        return 0;
    }

    KMINFO("Usage: work_queue status/start/stop\r\n");

    return 0;
}

#if !defined(CONFIG_MODULE_NSHLIB)
#if defined(CONFIG_GMSH)
MSH_FUNCTION_EXPORT_CMD(workqueue_main, work_queue, workqueue command.);
#endif

#if defined(CONFIG_FINSH)
MSH_CMD_EXPORT_ALIAS(workqueue_main, work_queue, workqueue command);
#endif
#endif
