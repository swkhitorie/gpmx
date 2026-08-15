/****************************************************************************
 * libs/libc/builtin/lib_builtin_forindex.c
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
#include <errno.h>
#include "builtin.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: builtin_for_index
 *
 * Description:
 *   Returns the builtin_s structure for the selected built-in.
 *   If support for built-in functions is enabled in the NuttX
 *   configuration, then this function must be provided by the application
 *   code.
 *
 * Input Parameters:
 *   index, from 0 and on...
 *
 * Returned Value:
 *   Returns valid pointer pointing to the builtin_s structure if index is
 *   valid.
 *   Otherwise, NULL is returned.
 *
 ****************************************************************************/

const struct builtin_s *builtin_for_index(int index)
{
    if (index < g_builtin_count) {
        return &g_builtins[index];
    }
    return NULL;
}

/****************************************************************************
 * Name: builtin_getname
 *
 * Description:
 *   Returns pointer the a name of the application at 'index' in the table
 *   of built-in applications.
 *
 * Input Parameters:
 *   index - From 0 and on ...
 *
 * Returned Value:
 *   Returns valid pointer pointing to the app name if index is valid.
 *   Otherwise NULL is returned.
 *
 ****************************************************************************/

const char *builtin_getname(int index)
{
    const struct builtin_s *builtin;
    builtin = builtin_for_index(index);
    if (builtin != NULL) {
        return builtin->name;
    }
    return NULL;
}

/****************************************************************************
 * Name: builtin_isavail
 *
 * Description:
 *   Checks for availability of an application named 'appname' registered
 *   during compile time and, if available, returns the index into the table
 *   of built-in applications.
 *
 * Input Parameters:
 *   filename - Name of the linked-in binary to be started.
 *
 * Returned Value:
 *   This is an internal function, used by by the NuttX binfmt logic and
 *   by the application built-in logic.  It returns a non-negative index to
 *   the application entry in the table of built-in applications on success
 *   or a negated errno value in the event of a failure.
 *
 ****************************************************************************/

int builtin_isavail(const char *appname)
{
    const char *name;
    int i;
    for (i = 0; (name = builtin_getname(i)) != NULL; i++) {
        if (strcmp(name, appname) == 0) {
            return i;
        }
    }
    return -ENOENT;
}
