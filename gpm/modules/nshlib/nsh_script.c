/****************************************************************************
 * apps/nshlib/nsh_script.c
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

#include "nsh.h"
#include "nsh_console.h"
#include <fcntl.h>

#if !defined(CONFIG_NSH_DISABLESCRIPT)

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static char *script_getline(int fd, char *buf, size_t size)
{
    size_t i = 0;
    char c;
    ssize_t n;

    if (size == 0) {
        return NULL;
    }

    while (i < size - 1) {
        n = read(fd, &c, 1);

        if (n == 0) {
            if (i == 0) {
                return NULL;
            }
            break;
        }

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return NULL;
        }

        buf[i++] = c;
        if (c == '\n') {
            break;
        }
    }

    buf[i] = '\0';
    return buf;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_script
 *
 * Description:
 *   Execute the NSH script at path.
 *
 ****************************************************************************/

int nsh_script(struct nsh_vtbl_s *vtbl, const char *cmd, const char *path)
{
    int   savefd;
    char *buffer;
    char *pret;
    int   ret = -1;

    /* Get a reference to the common input buffer */

    buffer = nsh_linebuffer(vtbl);
    if (buffer) {

        /* Save the parent fd in case of nested script processing */
        savefd = vtbl->np.np_fd;

        /* Open the file containing the script */
        vtbl->np.np_fd = open(path, O_RDONLY);
        if (vtbl->np.np_fd < 0) {
            nsh_error(vtbl, g_fmtcmdfailed, cmd, "open", NSH_ERRNO);
            vtbl->np.np_fd = savefd;
            return -1;
        }

        /* Loop, processing each command line in the script file (or
        * until an error occurs)
        */

        do {

#ifndef CONFIG_NSH_DISABLE_LOOPS
            /* Get the current file position.  This is used to control
            * looping.  If a loop begins in the next line, then this file
            * offset will be needed to locate the top of the loop in the
            * script file.  Note that ftell will return -1 on failure.
            */

            vtbl->np.np_foffs = lseek(vtbl->np.np_fd, 0, SEEK_CUR);
            vtbl->np.np_loffs = 0;

            if (vtbl->np.np_foffs < 0) {
                nsh_error(vtbl, g_fmtcmdfailed, "loop", "ftell", NSH_ERRNO);
            }
#endif

            /* Now read the next line from the script file */

            pret = script_getline(vtbl->np.np_fd, buffer, CONFIG_NSH_LINELEN);
            if (pret) {

                /* Parse process the command.  NOTE:  this is recursive...
                * we got to cmd_source via a call to nsh_parse.  So some
                * considerable amount of stack may be used.
                */

                if ((vtbl->np.np_flags & NSH_PFLAG_SILENT) == 0) {
                    nsh_output(vtbl, "%s", buffer);
                }

                ret = nsh_parse(vtbl, buffer);
            }

        } while (pret && (ret == 0 || (vtbl->np.np_flags & NSH_PFLAG_IGNORE)));

        /* Close the script file */

        close(vtbl->np.np_fd);

        /* Restore the parent script stream */

        vtbl->np.np_fd = savefd;
    }

    return ret;
}

/****************************************************************************
 * Name: nsh_initscript
 *
 * Description:
 *   Attempt to execute the configured initialization script.  This script
 *   should be executed once when NSH starts.  nsh_initscript is idempotent
 *   and may, however, be called multiple times (the script will be executed
 *   once.
 *
 ****************************************************************************/

int nsh_initscript(struct nsh_vtbl_s *vtbl)
{
    static bool initialized;
    bool already;
    int ret = 0;

    /* Atomic test and set of the initialized flag */

    // sched_lock();
    already     = initialized;
    initialized = true;
    // sched_unlock();

    /* If we have not already executed the init script, then do so now */

    if (!already) {

        ret = nsh_script(vtbl, "init", CONFIG_NSH_RUNPWD);
#ifndef CONFIG_NSH_DISABLESCRIPT
        /* Reset the option flags */

        vtbl->np.np_flags = NSH_NP_SET_OPTIONS_INIT;
#endif
    }

    return ret;
}

#endif /* !CONFIG_NSH_DISABLESCRIPT */
