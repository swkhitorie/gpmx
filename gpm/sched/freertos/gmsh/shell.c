#include <string.h>
#include <stdio.h>
#include <gpmx/config.h>

#include "shell.h"
#include "msh.h"

struct gmsh_syscall *_syscall_table_begin  = NULL;
struct gmsh_syscall *_syscall_table_end    = NULL;
struct gmsh_shell *shell;
static char *gmsh_prompt_custom = NULL;

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
struct gmsh_syscall *gmsh_syscall_next(struct gmsh_syscall *call)
{
    unsigned int *ptr;
    ptr = (unsigned int *)(call + 1);
    while ((*ptr == 0) && ((unsigned int *)ptr < (unsigned int *) _syscall_table_end))
        ptr ++;

    return (struct gmsh_syscall *)ptr;
}
#endif

int gmsh_set_prompt(const char *prompt)
{
    if (gmsh_prompt_custom) {
        // rt_free(gmsh_prompt_custom);
        gmsh_prompt_custom = NULL;
    }

    /* strdup */
    if (prompt) {
        // gmsh_prompt_custom = (char *)rt_malloc(strlen(prompt) + 1);
        if (gmsh_prompt_custom) {
            strcpy(gmsh_prompt_custom, prompt);
        }
    }

    return 0;
}

const char *gmsh_get_prompt()
{
    static char gmsh_prompt[255] = {0};

    if (!shell->prompt_mode) {
        gmsh_prompt[0] = '\0';
        return gmsh_prompt;
    }

    if (gmsh_prompt_custom) {
        strncpy(gmsh_prompt, gmsh_prompt_custom, sizeof(gmsh_prompt) - 1);
    } else {
        strcpy(gmsh_prompt, "msh ");
    }

    strcat(gmsh_prompt, ">");

    return gmsh_prompt;
}

int gmsh_get_prompt_mode()
{
    return shell->prompt_mode;
}

void gmsh_set_prompt_mode(int prompt)
{
    shell->prompt_mode = prompt;
}

int gmsh_getchar()
{
    extern char gmsh_console_getchar(void);
    return gmsh_console_getchar();
}

void gmsh_set_echo(int echo)
{
    shell->echo_mode = (uint8_t)echo;
}

int gmsh_get_echo()
{
    return shell->echo_mode;
}

static void shell_auto_complete(char *prefix)
{
    gsh_kprintf("\n");
    msh_auto_complete(prefix);

    gsh_kprintf("%s%s", gmsh_get_prompt(), prefix);
}

static bool shell_handle_history(struct gmsh_shell *shell)
{
    gsh_kprintf("\033[2K\r");
    gsh_kprintf("%s%s", gmsh_get_prompt(), shell->line);
    return false;
}

static void shell_push_history(struct gmsh_shell *shell)
{
    if (shell->line_position != 0) {

        if (shell->history_count >= CONFIG_GMSH_HISTORY_LINES) {

            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&shell->cmd_history[CONFIG_GMSH_HISTORY_LINES - 1], shell->line, CONFIG_GMSH_CMD_SIZE)) {
                /* move history */
                int index;
                for (index = 0; index < CONFIG_GMSH_HISTORY_LINES - 1; index ++) {
                    memcpy(&shell->cmd_history[index][0],
                        &shell->cmd_history[index + 1][0], CONFIG_GMSH_CMD_SIZE);
                }
                memset(&shell->cmd_history[index][0], 0, CONFIG_GMSH_CMD_SIZE);
                memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                /* it's the maximum history */
                shell->history_count = CONFIG_GMSH_HISTORY_LINES;
            }
        } else {

            /* if current cmd is same as last cmd, don't push */
            if (shell->history_count == 0 || memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, CONFIG_GMSH_CMD_SIZE)) {
                shell->current_history = shell->history_count;
                memset(&shell->cmd_history[shell->history_count][0], 0, CONFIG_GMSH_CMD_SIZE);
                memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                /* increase count and set current history position */
                shell->history_count ++;
            }
        }
    }
    shell->current_history = shell->history_count;
}

void gmsh_thread_entry(void *p)
{
    int ch;

    shell->echo_mode = 0;

    gsh_kprintf(gmsh_get_prompt());

    while (1) {

        ch = (int)gmsh_getchar();
        if (ch < 0) {
            continue;
        }

        /*
         * handle control key
         * up key  : 0x1b 0x5b 0x41
         * down key: 0x1b 0x5b 0x42
         * right key:0x1b 0x5b 0x43
         * left key: 0x1b 0x5b 0x44
         */
        if (ch == 0x1b) {

            shell->stat = WAIT_SPEC_KEY;
            continue;
        } else if (shell->stat == WAIT_SPEC_KEY) {

            if (ch == 0x5b) {
                shell->stat = WAIT_FUNC_KEY;
                continue;
            }

            shell->stat = WAIT_NORMAL;
        } else if (shell->stat == WAIT_FUNC_KEY) {

            shell->stat = WAIT_NORMAL;

            if (ch == 0x41) {

                /* prev history */
                if (shell->current_history > 0) {
                    shell->current_history --;
                } else {
                    shell->current_history = 0;
                    continue;
                }

                memcpy(shell->line, &shell->cmd_history[shell->current_history][0], CONFIG_GMSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = (uint16_t)strlen(shell->line);
                shell_handle_history(shell);
                continue;
            } else if (ch == 0x42) {

                /* next history */
                if (shell->current_history < shell->history_count - 1) {
                    shell->current_history ++;
                } else {
                    /* set to the end of history */
                    if (shell->history_count != 0) {
                        shell->current_history = shell->history_count - 1;
                    } else {
                        continue;
                    }
                }

                memcpy(shell->line, &shell->cmd_history[shell->current_history][0], CONFIG_GMSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = (uint16_t)strlen(shell->line);
                shell_handle_history(shell);
                continue;
            } else if (ch == 0x44) {
                if (shell->line_curpos) {
                    gsh_kprintf("\b");
                    shell->line_curpos --;
                }

                continue;
            } else if (ch == 0x43) {

                if (shell->line_curpos < shell->line_position) {
                    gsh_kprintf("%c", shell->line[shell->line_curpos]);
                    shell->line_curpos ++;
                }

                continue;
            }
        }

        /* received null or error */
        if (ch == '\0' || ch == 0xFF) {
            continue;
        } else if (ch == '\t') {

            int i;
            /* move the cursor to the beginning of line */
            for (i = 0; i < shell->line_curpos; i++) {
                gsh_kprintf("\b");
            }

            /* auto complete */
            shell_auto_complete(&shell->line[0]);
            /* re-calculate position */
            shell->line_curpos = shell->line_position = (uint16_t)strlen(shell->line);

            continue;
        } else if (ch == 0x7f || ch == 0x08) {

            /* handle backspace key */
            /* note that shell->line_curpos >= 0 */
            if (shell->line_curpos == 0) {
                continue;
            }

            shell->line_position--;
            shell->line_curpos--;

            if (shell->line_position > shell->line_curpos) {
                int i;

                memmove(&shell->line[shell->line_curpos],
                        &shell->line[shell->line_curpos + 1],
                        shell->line_position - shell->line_curpos);
                shell->line[shell->line_position] = 0;

                gsh_kprintf("\b%s  \b", &shell->line[shell->line_curpos]);

                /* move the cursor to the origin position */
                for (i = shell->line_curpos; i <= shell->line_position; i++) {
                    gsh_kprintf("\b");
                }
            } else {
                gsh_kprintf("\b \b");
                shell->line[shell->line_position] = 0;
            }

            continue;
        }

        /* handle end of line, break */
        if (ch == '\r' || ch == '\n') {
            shell_push_history(shell);
            if (shell->echo_mode) {
                gsh_kprintf("\n");
            }

            msh_exec(shell->line, shell->line_position);

            gsh_kprintf(gmsh_get_prompt());
            memset(shell->line, 0, sizeof(shell->line));
            shell->line_curpos = shell->line_position = 0;
            continue;
        }

        /* it's a large line, discard it */
        if (shell->line_position >= CONFIG_GMSH_CMD_SIZE) {
            shell->line_position = 0;
        }

        /* normal character */
        if (shell->line_curpos < shell->line_position) {
            int i;

            memmove(&shell->line[shell->line_curpos + 1],
                    &shell->line[shell->line_curpos],
                    shell->line_position - shell->line_curpos);
            shell->line[shell->line_curpos] = ch;
            if (shell->echo_mode) {
                gsh_kprintf("%s", &shell->line[shell->line_curpos]);
            }

            /* move the cursor to new position */
            for (i = shell->line_curpos; i < shell->line_position; i++) {
                gsh_kprintf("\b");
            }
        } else {
            shell->line[shell->line_position] = ch;
            if (shell->echo_mode) {
                gsh_kprintf("%c", ch);
            }
        }

        ch = 0;
        shell->line_position ++;
        shell->line_curpos++;
        if (shell->line_position >= CONFIG_GMSH_CMD_SIZE) {
            /* clear command line */
            shell->line_position = 0;
            shell->line_curpos = 0;
        }
    }
}

void gmsh_system_function_init(const void *begin, const void *end)
{
    _syscall_table_begin = (struct gmsh_syscall *) begin;
    _syscall_table_end = (struct gmsh_syscall *) end;
}

#if defined(__ICCARM__) || defined(__ICCRX__)

#pragma section="FSymTab"
#elif defined(__ADSPBLACKFIN__)

extern "asm" int __fsymtab_start;
extern "asm" int __fsymtab_end;
#elif defined(_MSC_VER)

#pragma section("FSymTab$a", read)
const char __fsym_begin_name[] = "__start";
const char __fsym_begin_desc[] = "begin of finsh";
__declspec(allocate("FSymTab$a")) const struct gmsh_syscall __fsym_begin =
{
    __fsym_begin_name,
    __fsym_begin_desc,
    NULL
};

#pragma section("FSymTab$z", read)
const char __fsym_end_name[] = "__end";
const char __fsym_end_desc[] = "end of finsh";
__declspec(allocate("FSymTab$z")) const struct gmsh_syscall __fsym_end =
{
    __fsym_end_name,
    __fsym_end_desc,
    NULL
};
#endif

int gmsh_system_init(void)
{
    int result = 0;

#ifdef __ARMCC_VERSION

    /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    gmsh_system_function_init(&FSymTab$$Base, &FSymTab$$Limit);
#elif defined (__ICCARM__) || defined(__ICCRX__)

    /* for IAR Compiler */
    gmsh_system_function_init(__section_begin("FSymTab"),
                               __section_end("FSymTab"));
#elif defined (__GNUC__) || defined(__TI_COMPILER_VERSION__) || defined(__TASKING__)

    /* GNU GCC Compiler and TI CCS */
    extern const int __fsymtab_start;
    extern const int __fsymtab_end;
    gmsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
#elif defined(__ADSPBLACKFIN__)

    /* for VisualDSP++ Compiler */
    gmsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
#elif defined(_MSC_VER)

    unsigned int *ptr_begin, *ptr_end;

    if (shell) {
        gsh_kprintf("finsh shell already init.\n");
        return 0;
    }

    ptr_begin = (unsigned int *)&__fsym_begin;
    ptr_begin += (sizeof(struct gmsh_syscall) / sizeof(unsigned int));
    while (*ptr_begin == 0) ptr_begin ++;

    ptr_end = (unsigned int *) &__fsym_end;
    ptr_end --;
    while (*ptr_end == 0) ptr_end --;

    gmsh_system_function_init(ptr_begin, ptr_end);
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
    shell = (struct gmsh_shell *)pvPortMalloc(sizeof(struct gmsh_shell));
    if (shell == NULL) {
        gsh_kprintf("no memory for shell\n");
        return -1;
    }

    if (pdPASS != xTaskCreate(gmsh_thread_entry, "gmsh", GSH_THREAD_STACK_SIZE/sizeof(BaseType_t), NULL, GSH_THREAD_PRIORITY, NULL)) {
        gsh_kprintf("gmsh task creat failed\n");
        return -2;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)
    shell = (struct gmsh_shell *)rt_calloc(1, sizeof(struct gmsh_shell));
    if (shell == RT_NULL) {
        gsh_kprintf("no memory for shell\n");
        return -1;
    }
    rt_thread_t tid = rt_thread_create("gmsh",
                            gmsh_thread_entry, RT_NULL,
                            GSH_THREAD_STACK_SIZE, GSH_THREAD_PRIORITY, 10);
    if (tid != NULL) {
        rt_thread_startup(tid);
    }
#endif

    gmsh_set_prompt_mode(1);
    gmsh_set_echo(1);

    return 0;
}

