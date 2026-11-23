#include <mlog.h>
#include "uorb_manager.h"
#include "uorb_device_master.h"
#include <string.h>

#if !defined(CONFIG_MODULE_NSHLIB)
#if defined(CONFIG_GMSH)
#include "gmsh.h"
#endif

#if defined(CONFIG_FINSH)
#include "shell.h"
#endif
#endif

int uorb_main(int argc, char **argv)
{
    if (argc != 2) {
        KMINFO("Usage: uorb status/start/stop\r\n");
        return 1;
    }

    if (!strcmp(argv[1], "start")) {
        return uorb_manager_initialize() ? 0 : 1;

    } else if (!strcmp(argv[1], "top")) {
        uorb_device_master_showtop(uorb_manager_get_device_master(uorb_manager_instance()), NULL, 0);
        return 0;

    } else if (!strcmp(argv[1], "status")) {
        uorb_device_master_print_statistics(uorb_manager_get_device_master(uorb_manager_instance()));
        return 0;
    }

    KMINFO("Usage: uorb status/start/stop\r\n");

    return 0;
}

#if !defined(CONFIG_MODULE_NSHLIB)
#if defined(CONFIG_GMSH)
MSH_FUNCTION_EXPORT_CMD(uorb_main, uorb, uorb command.);
#endif

#if defined(CONFIG_FINSH)
MSH_CMD_EXPORT_ALIAS(uorb_main, uorb, uorb command);
#endif
#endif
