#ifndef SHELL_H_
#define SHELL_H_

#include "gmsh.h"
#include <gpmx/config.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#endif

#ifndef GSH_THREAD_PRIORITY
#define GSH_THREAD_PRIORITY 16
#endif

#ifndef GSH_THREAD_STACK_SIZE
#define GSH_THREAD_STACK_SIZE 2048
#endif

#if !defined(CONFIG_GMSH_CMD_SIZE)
#define CONFIG_GMSH_CMD_SIZE     80
#endif

#if !defined(CONFIG_GMSH_HISTORY_LINES)
#define CONFIG_GMSH_HISTORY_LINES 5
#endif

enum input_stat {
    WAIT_NORMAL,
    WAIT_SPEC_KEY,
    WAIT_FUNC_KEY,
};

struct gmsh_shell {

    enum input_stat stat;

    uint8_t echo_mode;
    uint8_t prompt_mode;

    uint16_t current_history;
    uint16_t history_count;

    char cmd_history[CONFIG_GMSH_HISTORY_LINES][CONFIG_GMSH_CMD_SIZE];

    char line[CONFIG_GMSH_CMD_SIZE + 1];
    uint16_t line_position;
    uint16_t line_curpos;

};

#ifdef __cplusplus
extern "C" {
#endif

int  gmsh_system_init(void);

void gmsh_set_echo(int echo);
int  gmsh_get_echo(void);

int  gmsh_get_prompt_mode(void);
void gmsh_set_prompt_mode(int prompt);

const char *gmsh_get_prompt(void);
int gmsh_set_prompt(const char *prompt);

#ifdef __cplusplus
}
#endif


#endif
