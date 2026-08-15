/****************************************************************************
 * apps/builtin/exec_builtin.c
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

#include <pthread.h>
#include <stddef.h>
#include <errno.h>
#include "builtin.h"

#include "builtin_app.h"

/****************************************************************************
 * Name: exec_builtin
 *
 * Description:
 *   Executes builtin applications registered during 'make context' time.
 *   New application is run in a separate task context (and thread).
 *
 * Input Parameter:
 *   filename  - Name of the linked-in binary to be started.
 *   argv      - Argument list
 *   redirfile - If output is redirected, this parameter will be non-NULL
 *               and will provide the full path to the file.
 *   oflags    - If output is redirected, this parameter will provide the
 *               open flags to use.  This will support file replacement
 *               of appending to an existing file.
 *
 * Returned Value:
 *   This is an end-user function, so it follows the normal convention:
 *   Returns the PID of the exec'ed module.  On failure, it returns
 *   -1 (ERROR) and sets errno appropriately.
 *
 ****************************************************************************/

int exec_builtin(const char *appname, char * const *argv, bool joinable)
{
    pthread_attr_t attr;
    struct sched_param param;
    const struct builtin_s *builtin;
    int index;
    int ret;

    /* Verify that an application with this name exists */

    index = builtin_isavail(appname);
    if (index < 0) {
        ret = ENOENT;
        goto errout_with_errno;
    }

    /* Get information about the builtin */
    builtin = builtin_for_index(index);
    if (builtin == NULL) {
        ret = ENOENT;
        goto errout_with_errno;
    }

    int pid = built_task_spawn( builtin->name, SCHED_FIFO, builtin->priority,
                                builtin->stacksize, builtin->main,
                                (argv)?(&argv[0]):(char * const *)NULL, joinable);

    return pid;
errout_with_errno:
    errno = ret;
    return -1;
}
