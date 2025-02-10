#ifndef CLI_CONSOLE_H_
#define CLI_CONSOLE_H_

#include <stdint.h>
#include <stdio.h>

typedef int (*cli_cmd_t)(int argc, char **argv);

struct cli_cmdmap_s {
    const char *cmd;    /* Name of the command */
    cli_cmd_t   handler;
    uint8_t     minargs;
    uint8_t     maxargs;
    const char *usage;  /* Usage instructions for 'help' command */
};

#ifdef __cplusplus
extern "C" {
#endif

int cli_console_app_process();

#ifdef __cplusplus
}
#endif


#endif
