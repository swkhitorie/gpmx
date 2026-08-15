/****************************************************************************
 * apps/nshlib/nsh_envcmds.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "nsh.h"
#include "nsh_console.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CHAR_ESCAPE(s, c) \
          case (c):       \
            *(s)++ = (c); \
            break

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_ECHO
static void str_escape(char *s)
{
    char *q;
    int l;
    int c;

    for (q = s; *q; q++) {

        if (*q != '\\') {
            *s++ = *q;
            continue;
        }

        switch (*++q) {

        case '0':
            for (c = 0, l = 3; l && q[1] >= '0' && q[1] <= '8'; l--, q++) {
                c = 8 * c + (q[1] - '0');
            }

            *s++ = c;
            break;

        case 'x':
            for (c = 0, l = 2; l && isxdigit(q[1]); l--, q++) {
                if (isdigit(q[1])) {
                    c = 16 * c + (q[1] - '0');
                } else {
                    c = 16 * c + (tolower(q[1]) - 'a' + 10);
                }
            }

            *s++ = c;
            break;

        case '\0':
            *s++ = '\\';
            *s++ = '\0';
            return;

        default:
            *s++ = '\\';
            *s++ = *q;
            break;

            CHAR_ESCAPE(s, '\a');
            CHAR_ESCAPE(s, '\b');
            CHAR_ESCAPE(s, '\f');
            CHAR_ESCAPE(s, '\n');
            CHAR_ESCAPE(s, '\r');
            CHAR_ESCAPE(s, '\v');
            CHAR_ESCAPE(s, '\\');
        }
    }

    *s = '\0';
}
#endif

/****************************************************************************
 * Name: nsh_trimspaces
 *
 * Description:
 *   Trim any leading or trailing spaces from a string.
 *
 * Input Parameters:
 *   str - The string to be trimmed.  May be modified!
 *
 * Returned value:
 *   The new string pointer.
 *
 ****************************************************************************/

char *nsh_trimspaces(char *str)
{
    char *trimmed;
    int ndx;

    /* Strip leading whitespace from the value */

    for (trimmed = str;
        *trimmed != '\0' && isspace(*trimmed);
        trimmed++);

    /* Strip trailing whitespace from the value */

    for (ndx = strlen(trimmed) - 1;
        ndx >= 0 && isspace(trimmed[ndx]);
        ndx--)
    {
        trimmed[ndx] = '\0';
    }

    return trimmed;
}

/****************************************************************************
 * Name: nsh_dumpvar
 ****************************************************************************/

#if defined(CONFIG_NSH_VARS) && !defined(CONFIG_NSH_DISABLE_SET)
static int nsh_dumpvar(struct nsh_vtbl_s *vtbl, void *arg,
                       const char *pair)
{
    nsh_output(vtbl, "%s\n", pair);
    return 0;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: cmd_echo
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_ECHO
int cmd_echo(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    int newline = 1;
    int escape = 0;
    int opt;
    int i;

    while ((opt = getopt(argc, argv, "neE")) != -1) {
        switch (opt) {
        case 'n':
            newline = 0;
            break;

        case 'e':
            escape = 1;
            break;

        case 'E':
            escape = 0;
            break;

        case '?':
        default:
            nsh_error(vtbl, g_fmtarginvalid, argv[0]);
            return -1;
        }
    }

    /* echo each argument, separated by a space as it must have been on the
    * command line.
    */

    for (i = optind; i < argc; i++) {

        if (i != optind) {
            nsh_output(vtbl, " ");
        }

        if (escape) {
            str_escape(argv[i]);
        }

        nsh_output(vtbl, "%s", argv[i]);
    }

    if (newline) {
        nsh_output(vtbl, "\n");
    }

    return 0;
}
#endif

/****************************************************************************
 * Name: cmd_set
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_SET
int cmd_set(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    char *value;
    int ret = 0;
#ifdef NSH_HAVE_VARS
    int ndx = 1;
#endif

#ifndef CONFIG_NSH_DISABLESCRIPT
    char *popt;
    const char opts[] = NSH_NP_SET_OPTIONS;
    int op;

#ifdef CONFIG_NSH_VARS
    /* Set with no arguments will show all of the NSH variables */

    if (argc == 1) {
        ret = nsh_foreach_var(vtbl, nsh_dumpvar, NULL);
        nsh_output(vtbl, "\n");
        return ret < 0 ? -1 : 0;
    }
    else
#endif

#ifdef NSH_HAVE_VARS
    /* Support set [{+|-}{e|x|xe|ex}] [<name> <value>] */

    if (argc == 2 || argc == 4)
#else
    /* Support set [{+|-}{e|x|xe|ex}] */

#endif
    {
        if (strlen(argv[1]) < 2) {

            ret = -EINVAL;
            nsh_error(vtbl, g_fmtargrequired, argv[0], "set", NSH_ERRNO);
        } else {

            op = argv[1][0];
            if (op != '-' && op != '+') {
                ret = -EINVAL;
                nsh_error(vtbl, g_fmtarginvalid, argv[0], "set", NSH_ERRNO);
            } else {
                value = &argv[1][1];
                while (*value && *value != ' ') {

                    popt = strchr(opts, *value++);
                    if (popt == NULL) {
                        nsh_error(vtbl, g_fmtarginvalid,
                                  argv[0], "set", NSH_ERRNO);
                        ret = -EINVAL;
                        break;
                    }

                    if (op == '+') {
                        vtbl->np.np_flags |= 1 << (popt - opts);
                    } else {
                        vtbl->np.np_flags &= ~(1 << (popt - opts));
                    }
                }

#ifdef NSH_HAVE_VARS
                if (ret == 0) {
                    ndx = 2;
                }
#endif
            }
        }
    }

#ifdef NSH_HAVE_VARS
    if (ret == 0 && (argc == 3 || argc == 4))
#endif
#endif /* CONFIG_NSH_DISABLESCRIPT */

#ifdef NSH_HAVE_VARS
    {
#if defined(CONFIG_NSH_VARS) && !defined(CONFIG_DISABLE_ENVIRON)
        char *oldvalue;
#endif

        /* Trim whitespace from the value */

        value = nsh_trimspaces(argv[ndx + 1]);

#ifdef CONFIG_NSH_VARS
#ifndef CONFIG_DISABLE_ENVIRON
        /* Check if the NSH variable has already been promoted to an group-
        * wide environment variable.
        *
        * REVISIT:  Is this the correct behavior?  Bash would create/modify
        * a local variable that shadows the environment variable.
        */

        oldvalue = getenv(argv[ndx]);
        if (oldvalue == NULL)
#endif
        {
            /* Set the NSH variable */

            ret = nsh_setvar(vtbl, argv[ndx], value);
            if (ret < 0) {
                nsh_error(vtbl, g_fmtcmdfailed, argv[0], "nsh_setvar",
                          NSH_ERRNO_OF(-ret));
            }
        }
#endif /* CONFIG_NSH_VARS */

    }
#endif /* NSH_HAVE_VARS */

    return ret;
}
#endif /* CONFIG_NSH_DISABLE_SET */

/****************************************************************************
 * Name: cmd_unset
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_UNSET
int cmd_unset(struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
#if defined(CONFIG_NSH_VARS) || !defined(CONFIG_DISABLE_ENVIRON)
    int status;
#endif
    int ret = 0;

#if defined(CONFIG_NSH_VARS)
    /* Unset NSH variable */

    status = nsh_unsetvar(vtbl, argv[1]);
    if (status < 0 && status != -ENOENT) {
        nsh_error(vtbl, g_fmtcmdfailed, argv[0], "nsh_unsetvar",
                  NSH_ERRNO_OF(-status));
        ret = -1;
    }
#endif

#if !defined(CONFIG_DISABLE_ENVIRON)
  /* Unset environment variable */

    status = unsetenv(argv[1]);
    if (status < 0) {
        nsh_error(vtbl, g_fmtcmdfailed, argv[0], "unsetenv", NSH_ERRNO);
        ret = -1;
    }
#endif

    return ret;
}
#endif
