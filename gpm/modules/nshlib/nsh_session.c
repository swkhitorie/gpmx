/****************************************************************************
 * apps/nshlib/nsh_session.c
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
#include <string.h>
#include <assert.h>

#include "readline/readline.h"

#include "nsh.h"
#include "nsh_console.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_session
 *
 * Description:
 *   This is the common session login on any NSH session.  This function
 *   returns when an error reading from the input stream occurs, presumably
 *   signaling the end of the session.
 *
 *   This function:
 *   - Performs the login sequence if so configured
 *   - Executes the NSH login script
 *   - Presents a greeting
 *   - Then provides a prompt then gets and processes the command line.
 *   - This continues until an error occurs, then the session returns.
 *
 * Input Parameters:
 *   pstate - Abstracts the underlying session.
 *
 * Returned Values:
 *   EXIT_SUCCESS or EXIT_FAILURE is returned.
 *
 ****************************************************************************/

int nsh_session(struct console_stdio_s *pstate,
                bool login, int argc, char *argv[])
{
    struct nsh_vtbl_s *vtbl;
    int ret = EXIT_FAILURE;

    vtbl = &pstate->cn_vtbl;

    if (login) {
        /* Present a greeting and possibly a Message of the Day (MOTD) */
        nsh_output(vtbl, "%s", g_nshgreeting);
    }

    /* Process the command line option */

    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0) {

            nsh_output(vtbl, "Usage: %s [<script-path>|-c <command>]\n",
                        argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "-c") == 0) {

            /* Process the inline command */
            if (argc > 2) {
                return nsh_parse(vtbl, argv[2]);
            } else {
                nsh_error(vtbl, g_fmtargrequired, argv[0]);
                return EXIT_FAILURE;
            }
        } else if (argv[1][0] == '-') {

            /* Unknown option */
            nsh_error(vtbl, g_fmtsyntax, argv[0]);
            return EXIT_FAILURE;
        } else {

#ifndef CONFIG_NSH_DISABLESCRIPT
            /* Execute the shell script */
            return nsh_script(vtbl, argv[0], argv[1]);
#else
            return EXIT_FAILURE;
#endif
        }
    }

    /* Then enter the command line parsing loop */
    for (; ;) {

        /* Get the next line of input. readline() returns EOF
        * on end-of-file or any read failure.
        */

        /* Display the prompt string */
        nsh_output(vtbl, "%s", g_nshprompt);

        /* readline() normally returns the number of characters read, but
        * will return EOF on end of file or if an error occurs.  EOF
        * will cause the session to terminate.
        */

        // pstate->cn_line not clear;
        ret = readline(pstate->cn_line, CONFIG_NSH_LINELEN,
                        STDIN_FILENO, STDOUT_FILENO);

        if (ret == EOF) {
            /* NOTE: readline() does not set the errno variable, but
            * perhaps we will be lucky and it will still be valid.
            */

            nsh_error(vtbl, "%s nsh_session readline", g_fmtcmdfailed);
            ret = EXIT_SUCCESS;
            break;
        }

        /* Parse process the command */
        nsh_parse(vtbl, pstate->cn_line);
    }

    return ret;
}
