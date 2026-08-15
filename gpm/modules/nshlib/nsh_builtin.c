/****************************************************************************
 * apps/nshlib/nsh_builtin.c
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

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "nsh.h"
#include "nsh_console.h"

#include <builtin/builtin_app.h>

#ifdef CONFIG_NSH_BUILTIN_APPS

/****************************************************************************
 * Name: nsh_builtin
 *
 * Description:
 *    Attempt to execute the application task whose name is 'cmd'
 *
 * Returned Value:
 *   <0          If exec_builtin() fails, then the negated errno value
 *               is returned.
 *   -1 (ERROR)  if the application task corresponding to 'cmd' could not
 *               be started (possibly because it doesn not exist).
 *    0 (OK)     if the application task corresponding to 'cmd' was
 *               and successfully started.  If CONFIG_SCHED_WAITPID is
 *               defined, this return value also indicates that the
 *               application returned successful status (EXIT_SUCCESS)
 *    1          If CONFIG_SCHED_WAITPID is defined, then this return value
 *               indicates that the application task was spawned successfully
 *               but returned failure exit status.
 *
 ****************************************************************************/

int nsh_builtin(struct nsh_vtbl_s *vtbl, const char *cmd, char **argv)
{
    int ret = 0;
    const struct builtin_s *builtin;

    ret = exec_builtin(cmd, argv, true);

    if (ret >= 0) {
        built_task_join(ret);
    } else {
        // error 
    }

    return ret;
}

#endif /* CONFIG_NSH_BUILTIN_APPS */
