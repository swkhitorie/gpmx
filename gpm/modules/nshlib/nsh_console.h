/****************************************************************************
 * apps/nshlib/nsh_console.h
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

#ifndef __APPS_NSHLIB_NSH_CONSOLE_H
#define __APPS_NSHLIB_NSH_CONSOLE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Method access macros */

#define nsh_clone(v)           (v)->clone(v)
#define nsh_release(v)         (v)->release(v)
#define nsh_linebuffer(v)      (v)->linebuffer(v)
#define nsh_exit(v,s)          (v)->exit(v,s)

#ifdef CONFIG_CPP_HAVE_VARARGS
#define nsh_error(v, ...)     (v)->error(v, ##__VA_ARGS__)
#define nsh_output(v, ...)    (v)->output(v, ##__VA_ARGS__)
#else
#define nsh_error             vtbl->error
#define nsh_output            vtbl->output
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* This describes a generic console front-end */

struct nsh_vtbl_s
{
    /* This function pointers are "hooks" into the front end logic to
    * handle things like output of command results, redirection, etc.
    * -- all of which must be done in a way that is unique to the nature
    * of the front end.
    */

#ifndef CONFIG_NSH_DISABLEBG
    struct nsh_vtbl_s *(*clone)(struct nsh_vtbl_s *vtbl);
#endif
    void  (*release)(struct nsh_vtbl_s *vtbl);
    int   (*error)(struct nsh_vtbl_s *vtbl, const char *fmt, ...);
    int   (*output)(struct nsh_vtbl_s *vtbl, const char *fmt, ...);
    char *(*linebuffer)(struct nsh_vtbl_s *vtbl);
    void  (*exit)(struct nsh_vtbl_s *vtbl, int status);

    /* Parser state data */
    struct nsh_parser_s np;
};

/* This structure describes a console front-end that is based on stdin and
 * stdout (which is all of the supported console types at the time being).
 */

struct console_stdio_s
{
    /* NSH front-end call table */
    struct nsh_vtbl_s cn_vtbl;

#ifdef CONFIG_NSH_VARS
    /* Allocation and size of NSH variables */
    char *varp;
    size_t varsz;
#endif

    /* Line input buffer */
    char   cn_line[CONFIG_NSH_LINELEN];
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Defined in nsh_console.c *************************************************/

struct console_stdio_s *nsh_newconsole();

#endif /* __APPS_NSHLIB_NSH_CONSOLE_H */
