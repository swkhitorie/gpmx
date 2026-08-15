/****************************************************************************
 * apps/nshlib/nsh_command.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <string.h>
#include "nsh.h"
#include "nsh_console.h"

#ifdef CONFIG_NSH_BUILTIN_APPS
#include <builtin/builtin.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Help command summary layout */

#define HELP_LINELEN  80
#define NUM_CMDS      ((sizeof(g_cmdmap)/sizeof(struct cmdmap_s)) - 1)

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct cmdmap_s
{
  const char *cmd;        /* Name of the command */
  nsh_cmd_t   handler;    /* Function that handles the command */
  uint8_t     minargs;    /* Minimum number of arguments (including command) */
  uint8_t     maxargs;    /* Maximum number of arguments (including command) */
  const char *usage;      /* Usage instructions for 'help' command */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_HELP
static int  cmd_help(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#ifndef CONFIG_NSH_DISABLESCRIPT
static int  cmd_true(struct nsh_vtbl_s *vtbl, int argc, char **argv);
static int  cmd_false(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#ifndef CONFIG_NSH_DISABLE_EXIT
static int  cmd_exit(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

static int  cmd_unrecognized(struct nsh_vtbl_s *vtbl, int argc,
                             char **argv);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct cmdmap_s g_cmdmap[] =
{
#if defined(CONFIG_FILE_STREAM) && !defined(CONFIG_NSH_DISABLESCRIPT)
#ifndef CONFIG_NSH_DISABLE_SOURCE
    { ".",        cmd_source,   2, 2, "<script-path>" },
#endif
#endif

#if !defined(CONFIG_NSH_DISABLESCRIPT) && !defined(CONFIG_NSH_DISABLE_TEST)
    { "[",        cmd_lbracket, 4, CONFIG_NSH_MAXARGUMENTS, "<expression> ]" },
#endif

#ifndef CONFIG_NSH_DISABLE_HELP
    { "?",        cmd_help,     1, 1, NULL },
#endif

#if !defined(CONFIG_NSH_DISABLESCRIPT) && !defined(CONFIG_NSH_DISABLE_LOOPS)
    { "break",     cmd_break,   1, 1, NULL },
#endif

#ifndef CONFIG_NSH_DISABLE_ECHO
    { "echo",     cmd_echo,     1, CONFIG_NSH_MAXARGUMENTS,
      "[-n] [<string|$name> [<string|$name>...]]" },
#endif

#ifndef CONFIG_NSH_DISABLE_EXEC
    { "exec",     cmd_exec,     2, 3, "<hex-address>" },
#endif

#ifndef CONFIG_NSH_DISABLE_EXIT
    { "exit",     cmd_exit,     1, 1, NULL },
#endif

#ifndef CONFIG_NSH_DISABLESCRIPT
    { "false",    cmd_false,    1, 1, NULL },
#endif

#ifdef CONFIG_DEBUG_MM
#ifndef CONFIG_NSH_DISABLE_MEMDUMP
    { "memdump",  cmd_memdump,  1, 3, "[pid/used/free/on/off]" },
#endif
#endif

#ifndef CONFIG_NSH_DISABLE_HELP
#ifdef CONFIG_NSH_HELP_TERSE
    { "help",     cmd_help,     1, 2, "[<cmd>]" },
#else
    { "help",     cmd_help,     1, 3, "[-v] [<cmd>]" },
#endif
#endif

#ifndef CONFIG_NSH_DISABLE_SET
#ifdef CONFIG_NSH_VARS

#if !defined(CONFIG_DISABLE_ENVIRON) && !defined(CONFIG_NSH_DISABLESCRIPT)
    { "set",      cmd_set,      1, 4, "[{+|-}{e|x|xe|ex}] [<name> <value>]" },
#elif !defined(CONFIG_DISABLE_ENVIRON) && defined(CONFIG_NSH_DISABLESCRIPT)
    { "set",      cmd_set,      1, 3, "[<name> <value>]" },
#elif defined(CONFIG_DISABLE_ENVIRON) && !defined(CONFIG_NSH_DISABLESCRIPT)
    { "set",      cmd_set,      1, 2, "[{+|-}{e|x|xe|ex}]" },
#endif

#else

#if !defined(CONFIG_DISABLE_ENVIRON) && !defined(CONFIG_NSH_DISABLESCRIPT)
    { "set",      cmd_set,      2, 4, "[{+|-}{e|x|xe|ex}] [<name> <value>]" },
#elif !defined(CONFIG_DISABLE_ENVIRON) && defined(CONFIG_NSH_DISABLESCRIPT)
    { "set",      cmd_set,      3, 3, "<name> <value>" },
#elif defined(CONFIG_DISABLE_ENVIRON) && !defined(CONFIG_NSH_DISABLESCRIPT)
    { "set",      cmd_set,      2, 2, "{+|-}{e|x|xe|ex}" },
#endif

#endif
#endif /* CONFIG_NSH_DISABLE_SET */

#if defined(CONFIG_FILE_STREAM) && !defined(CONFIG_NSH_DISABLESCRIPT)
#ifndef CONFIG_NSH_DISABLE_SOURCE
    { "source",   cmd_source,   2, 2, "<script-path>" },
#endif
#endif

#if !defined(CONFIG_NSH_DISABLESCRIPT) && !defined(CONFIG_NSH_DISABLE_TEST)
    { "test",     cmd_test,     3, CONFIG_NSH_MAXARGUMENTS, "<expression>" },
#endif

#ifndef CONFIG_NSH_DISABLESCRIPT
    { "true",     cmd_true,     1, 1, NULL },
#endif

#ifndef CONFIG_NSH_DISABLE_UNSET
    { "unset",    cmd_unset,    2, 2, "<name>" },
#endif

    { NULL,       NULL,         1, 1, NULL }
};

/****************************************************************************
 * Name: help_cmdlist
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_HELP
static inline void help_cmdlist(struct nsh_vtbl_s *vtbl)
{
    unsigned int colwidth;
    unsigned int cmdwidth;
    unsigned int cmdsperline;
    unsigned int ncmdrows;
    unsigned int i;
    unsigned int j;
    unsigned int k;

    /* Pick an optimal column width */

    for (k = 0, colwidth = 0; k < NUM_CMDS; k++) {
        cmdwidth = strlen(g_cmdmap[k].cmd);
        if (cmdwidth > colwidth) {
            colwidth = cmdwidth;
        }
      }

    colwidth += 2;

    /* Determine the number of commands to put on one line */

    if (colwidth > HELP_LINELEN) {
        cmdsperline = 1;
    } else {
        cmdsperline = HELP_LINELEN / colwidth;
    }

    /* Determine the total number of lines to output */

    ncmdrows = (NUM_CMDS + (cmdsperline - 1)) / cmdsperline;

    /* Print the command name in 'ncmdrows' rows with 'cmdsperline' commands
    * on each line.
    */

    for (i = 0; i < ncmdrows; i++) {
        nsh_output(vtbl, "  ");
        for (j = 0, k = i;
            j < cmdsperline && k < NUM_CMDS;
            j++, k += ncmdrows)
        {
            nsh_output(vtbl, "%s", g_cmdmap[k].cmd);

            for (cmdwidth = strlen(g_cmdmap[k].cmd);
                cmdwidth < colwidth;
                cmdwidth++)
            {
                nsh_output(vtbl, " ");
            }
        }

        nsh_output(vtbl, "\n");
    }
}
#endif

/****************************************************************************
 * Name: help_usage
 ****************************************************************************/

#if !defined(CONFIG_NSH_DISABLE_HELP) && !defined(CONFIG_NSH_HELP_TERSE)
static inline void help_usage(struct nsh_vtbl_s *vtbl)
{
    nsh_output(vtbl, "NSH command forms:\n");
#ifndef CONFIG_NSH_DISABLEBG
    nsh_output(vtbl, "  [nice [-d <niceness>>]] <cmd> "
                    "[> <file>|>> <file>] [&]\n\n");
#else
    nsh_output(vtbl, "  <cmd> [> <file>|>> <file>]\n\n");
#endif
#ifndef CONFIG_NSH_DISABLESCRIPT
#ifndef CONFIG_NSH_DISABLE_ITEF
    nsh_output(vtbl, "OR\n");
    nsh_output(vtbl, "  if <cmd>\n");
    nsh_output(vtbl, "  then\n");
    nsh_output(vtbl, "    [sequence of <cmd>]\n");
    nsh_output(vtbl, "  else\n");
    nsh_output(vtbl, "    [sequence of <cmd>]\n");
    nsh_output(vtbl, "  fi\n\n");
#endif
#ifndef CONFIG_NSH_DISABLE_LOOPS
    nsh_output(vtbl, "OR\n");
    nsh_output(vtbl, "  while <cmd>\n");
    nsh_output(vtbl, "  do\n");
    nsh_output(vtbl, "    [sequence of <cmd>]\n");
    nsh_output(vtbl, "  done\n\n");
    nsh_output(vtbl, "OR\n");
    nsh_output(vtbl, "  until <cmd>\n");
    nsh_output(vtbl, "  do\n");
    nsh_output(vtbl, "    [sequence of <cmd>]\n");
    nsh_output(vtbl, "  done\n\n");
#endif
#endif
}
#endif

/****************************************************************************
 * Name: help_showcmd
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_HELP
static void help_showcmd(struct nsh_vtbl_s *vtbl,
                         const struct cmdmap_s *cmdmap)
{
    if (cmdmap->usage) {
        nsh_output(vtbl, "  %s %s\n", cmdmap->cmd, cmdmap->usage);
    } else {
        nsh_output(vtbl, "  %s\n", cmdmap->cmd);
    }
}
#endif

/****************************************************************************
 * Name: help_cmd
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_HELP
static int help_cmd(struct nsh_vtbl_s *vtbl, const char *cmd)
{
    const struct cmdmap_s *cmdmap;

    /* Find the command in the command table */

    for (cmdmap = g_cmdmap; cmdmap->cmd; cmdmap++) {
        /* Is this the one we are looking for? */

        if (strcmp(cmdmap->cmd, cmd) == 0) {
            /* Yes... show it */

            nsh_output(vtbl, "%s usage:", cmd);
            help_showcmd(vtbl, cmdmap);
            return 0;
        }
    }

    nsh_error(vtbl, g_fmtcmdnotfound, cmd);
    return -1;
}
#endif

/****************************************************************************
 * Name: help_allcmds
 ****************************************************************************/

#if !defined(CONFIG_NSH_DISABLE_HELP) && !defined(CONFIG_NSH_HELP_TERSE)
static inline void help_allcmds(struct nsh_vtbl_s *vtbl)
{
    const struct cmdmap_s *cmdmap;

    /* Show all of the commands in the command table */

    for (cmdmap = g_cmdmap; cmdmap->cmd; cmdmap++) {
        help_showcmd(vtbl, cmdmap);
    }
}
#endif

/****************************************************************************
 * Name: help_builtins
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_HELP
static inline void help_builtins(struct nsh_vtbl_s *vtbl)
{
#ifdef CONFIG_NSH_BUILTIN_APPS
    const struct builtin_s *builtin;
    unsigned int builtins_per_line;
    unsigned int num_builtin_rows;
    unsigned int builtin_width;
    unsigned int num_builtins;
    unsigned int column_width;
    unsigned int i;
    unsigned int j;
    unsigned int k;

    /* Count the number of built-in commands and get the optimal column width */

    num_builtins = 0;
    column_width = 0;

    for (i = 0; (builtin = builtin_for_index(i)) != NULL; i++) {

        if (builtin->main == NULL) {
            continue;
        }

        num_builtins++;

        builtin_width = strlen(builtin->name);
        if (builtin_width > column_width) {
            column_width = builtin_width;
        }
    }

    /* Skip the printing if no available built-in commands */

    if (num_builtins == 0) {
        return;
    }

    column_width += 2;

    /* Determine the number of commands to put on one line */

    if (column_width > HELP_LINELEN) {
        builtins_per_line = 1;
    } else {
        builtins_per_line = HELP_LINELEN / column_width;
    }

    /* Determine the total number of lines to output */

    num_builtin_rows = ((num_builtins + (builtins_per_line - 1)) /
                        builtins_per_line);

    /* List the set of available built-in commands */

    nsh_output(vtbl, "\nBuiltin Apps:\n");
    for (i = 0; i < num_builtin_rows; i++) {
        nsh_output(vtbl, "  ");
        for (j = 0, k = i;
            j < builtins_per_line &&
            (builtin = builtin_for_index(k));
            j++, k += num_builtin_rows)
        {
            if (builtin->main == NULL) {
                continue;
            }

            nsh_output(vtbl, "%s", builtin->name);

            for (builtin_width = strlen(builtin->name);
                builtin_width < column_width;
                builtin_width++)
            {
                nsh_output(vtbl, " ");
            }
        }

        nsh_output(vtbl, "\n");
    }
#endif
}
#endif

/****************************************************************************
 * Name: cmd_help
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_HELP
static int cmd_help(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    const char *cmd = NULL;
#ifndef CONFIG_NSH_HELP_TERSE
    bool verbose = false;
    int i;
#endif

    /* The command may be followed by a verbose option */

#ifndef CONFIG_NSH_HELP_TERSE
    i = 1;
    if (argc > i) {

        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
            i++;
        }
    }

    /* The command line may end with a command name */

    if (argc > i) {
        cmd = argv[i];
    }

    /* Show the generic usage if verbose is requested */

    if (verbose) {
        help_usage(vtbl);
    }
#else
    if (argc > 1) {
        cmd = argv[1];
    }
#endif

    /* Are we showing help on a single command? */

    if (cmd) {
        /* Yes.. show the single command */

        help_cmd(vtbl, cmd);
    } else {

        /* In verbose mode, show detailed help for all commands */

#ifndef CONFIG_NSH_HELP_TERSE
        if (verbose) {
            nsh_output(vtbl, "Where <cmd> is one of:\n");
            help_allcmds(vtbl);
        }

        /* Otherwise, just show the list of command names */

        else
#endif
        {
            help_cmd(vtbl, "help");
            nsh_output(vtbl, "\n");
            help_cmdlist(vtbl);
        }

        /* And show the list of built-in applications */

        help_builtins(vtbl);
    }

    return 0;
}
#endif

/****************************************************************************
 * Name: cmd_unrecognized
 ****************************************************************************/

static int cmd_unrecognized(struct nsh_vtbl_s *vtbl, int argc,
                            char **argv)
{
      nsh_error(vtbl, g_fmtcmdnotfound, argv[0]);
      return -1;
}

/****************************************************************************
 * Name: cmd_true
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLESCRIPT
static int cmd_true(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    return 0;
}
#endif

/****************************************************************************
 * Name: cmd_false
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLESCRIPT
static int cmd_false(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    return -1;
}
#endif

/****************************************************************************
 * Name: cmd_exit
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_EXIT
static int cmd_exit(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    nsh_exit(vtbl, 0);
    return 0;
}
#endif

/****************************************************************************
 * Name: nsh_command
 *
 * Description:
 *   Execute the command in argv[0]
 *
 * Returned Value:
 *   -1 (ERROR) if the command was unsuccessful
 *    0 (OK)     if the command was successful
 *
 ****************************************************************************/

int nsh_command(struct nsh_vtbl_s *vtbl, int argc, char *argv[])
{
    const struct cmdmap_s *cmdmap;
    const char            *cmd;
    nsh_cmd_t              handler = cmd_unrecognized;
    int                    ret;

    /* The form of argv is:
    *
    * argv[0]:      The command name.  This is argv[0] when the arguments
    *               are, finally, received by the command vtblr
    * argv[1]:      The beginning of argument (up to CONFIG_NSH_MAXARGUMENTS)
    * argv[argc]:   NULL terminating pointer
    */

    cmd = argv[0];

    /* See if the command is one that we understand */

    for (cmdmap = g_cmdmap; cmdmap->cmd; cmdmap++) {

        if (strcmp(cmdmap->cmd, cmd) == 0) {
            /* Check if a valid number of arguments was provided.  We
            * do this simple, imperfect checking here so that it does
            * not have to be performed in each command.
            */

            if (argc < cmdmap->minargs) {
                /* Fewer than the minimum number were provided */

                nsh_error(vtbl, g_fmtargrequired, cmd);
                return -1;
            } else if (argc > cmdmap->maxargs) {
                /* More than the maximum number were provided */

                nsh_error(vtbl, g_fmttoomanyargs, cmd);
                return -1;
            } else {
                /* A valid number of arguments were provided (this does
                * not mean they are right).
                */

                handler = cmdmap->handler;
                break;
            }
        }
    }

    ret = handler(vtbl, argc, argv);
    return ret;
}

int cmd_exec(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    return 0;
}
