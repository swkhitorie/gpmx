#include <string.h>
#include "msh.h"
#include "shell.h"
#include <gpmx/config.h>

#if !defined(CONFIG_GMSH_ARG_MAX)
#define CONFIG_GMSH_ARG_MAX    8
#endif

int msh_help(int argc, char **argv)
{
    gsh_kprintf("GPMX shell commands:\n");
    {
        struct gmsh_syscall *index;

        for (index = _syscall_table_begin; index < _syscall_table_end;
            GMSH_NEXT_SYSCALL(index))
        {
            gsh_kprintf("%-16s - %s\n", index->name, index->desc);
        }
    }
    gsh_kprintf("\n");

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(msh_help, help, GPMX shell help.);

static int msh_split(char *cmd, int length, char *argv[CONFIG_GMSH_ARG_MAX])
{
    char *ptr;
    int position;
    int argc;
    int i;

    ptr = cmd;
    position = 0;
    argc = 0;

    while (position < length) {

        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && position < length) {
            *ptr = '\0';
            ptr ++;
            position ++;
        }

        if (argc >= CONFIG_GMSH_ARG_MAX) {

            gsh_kprintf("Too many args ! We only Use:\n");
            for (i = 0; i < argc; i++) {
                gsh_kprintf("%s ", argv[i]);
            }
            gsh_kprintf("\n");
            break;
        }

        if (position >= length) {
            break;
        }

        /* handle string */
        if (*ptr == '"') {

            ptr ++;
            position ++;
            argv[argc] = ptr;
            argc ++;

            /* skip this string */
            while (*ptr != '"' && position < length) {

                if (*ptr == '\\') {

                    if (*(ptr + 1) == '"') {

                        ptr ++;
                        position ++;
                    }
                }
                ptr ++;
                position ++;
            }

            if (position >= length) {
                break;
            }

            /* skip '"' */
            *ptr = '\0';
            ptr ++;
            position ++;
        } else {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length) {
                ptr ++;
                position ++;
            }

            if (position >= length) {
                break;
            }
        }
    }

    return argc;
}

static gmsh_syscall_func msh_get_cmd(char *cmd, int size)
{
    struct gmsh_syscall *index;
    gmsh_syscall_func cmd_func = NULL;

    for (index = _syscall_table_begin; index < _syscall_table_end; GMSH_NEXT_SYSCALL(index)) {
        if (strncmp(index->name, cmd, size) == 0 && index->name[size] == '\0') {
            cmd_func = index->func;
            break;
        }
    }

    return cmd_func;
}

static int _msh_exec_cmd(char *cmd, int length, int *retp)
{
    int argc;
    int cmd0_size = 0;
    gmsh_syscall_func cmd_func;
    char *argv[CONFIG_GMSH_ARG_MAX];

    while ((cmd[cmd0_size] != ' ' && cmd[cmd0_size] != '\t') && cmd0_size < length) {
        cmd0_size ++;
    }

    if (cmd0_size == 0) {
        return -1;
    }

    cmd_func = msh_get_cmd(cmd, cmd0_size);
    if (cmd_func == NULL) {
        return -1;
    }

    memset(argv, 0x00, sizeof(argv));
    argc = msh_split(cmd, length, argv);
    if (argc == 0) {
        return -1;
    }

    *retp = cmd_func(argc, argv);

    return 0;
}

int msh_exec(char *cmd, int length)
{
    int cmd_ret;

    while ((length > 0) && (*cmd  == ' ' || *cmd == '\t')) {
        cmd++;
        length--;
    }

    if (length == 0) {
        return 0;
    }

    if (_msh_exec_cmd(cmd, length, &cmd_ret) == 0) {
        return cmd_ret;
    }

    /* truncate the cmd at the first space. */
    {
        char *tcmd;
        tcmd = cmd;
        while (*tcmd != ' ' && *tcmd != '\0') {
            tcmd++;
        }
        *tcmd = '\0';
    }
    gsh_kprintf("%s: command not found.\n", cmd);
    return -1;
}

static int str_common(const char *str1, const char *str2)
{
    const char *str = str1;

    while ((*str != 0) && (*str2 != 0) && (*str == *str2)) {
        str ++;
        str2 ++;
    }

    return (str - str1);
}

void msh_auto_complete(char *prefix)
{
    int length, min_length;
    const char *name_ptr, *cmd_name;
    struct gmsh_syscall *index;

    min_length = 0;
    name_ptr = NULL;

    if (*prefix == '\0') {
        msh_help(0, NULL);
        return;
    }

    /* checks in internal command */
    {
        for (index = _syscall_table_begin; index < _syscall_table_end; GMSH_NEXT_SYSCALL(index)) {

            cmd_name = (const char *) index->name;
            if (strncmp(prefix, cmd_name, strlen(prefix)) == 0) {

                if (min_length == 0) {
                    name_ptr = cmd_name;
                    min_length = strlen(name_ptr);
                }

                length = str_common(name_ptr, cmd_name);
                if (length < min_length) {
                    min_length = length;
                }

                gsh_kprintf("%s\n", cmd_name);
            }
        }
    }

    /* auto complete string */
    if (name_ptr != NULL) {
        strncpy(prefix, name_ptr, min_length);
    }

    return;
}
