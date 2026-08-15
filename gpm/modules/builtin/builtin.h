/****************************************************************************
 * include/nuttx/lib/builtin.h
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

#ifndef __INCLUDE_NUTTX_LIB_BUILTIN_H
#define __INCLUDE_NUTTX_LIB_BUILTIN_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

typedef int (*main_t)(int argc, char *argv[]);

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct builtin_s
{
  const char *name;         /* Invocation name and as seen under /sbin/ */
  int         priority;     /* Use: SCHED_PRIORITY_DEFAULT */
  int         stacksize;    /* Desired stack size */
  main_t      main;         /* Entry point: main(int argc, char *argv[]) */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* In the FLAT build, the builtin list is just a global global array and
 * count exported from user space via the backdoor left open by the FLAT
 * address space.  These globals must be provided the application layer.
 */

EXTERN const struct builtin_s g_builtins[];
EXTERN const int g_builtin_count;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

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

int builtin_isavail(const char *appname);

/****************************************************************************
 * Name: builtin_getname
 *
 * Description:
 *   Returns pointer to a name of built-in application pointed by the
 *   index.
 *
 * Input Parameters:
 *   index, from 0 and on ...
 *
 * Returned Value:
 *   Returns valid pointer pointing to the app name if index is valid.
 *   Otherwise NULL is returned.
 *
 ****************************************************************************/

const char *builtin_getname(int index);

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

const struct builtin_s *builtin_for_index(int index);

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

int exec_builtin(const char *appname, char * const *argv, bool joinable);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif

