/****************************************************************************
 * apps/nshlib/nsh_console.c
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "nsh.h"
#include "nsh_console.h"

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLEBG
static struct nsh_vtbl_s *nsh_consoleclone(struct nsh_vtbl_s *vtbl);
#endif
static void  nsh_consolerelease(struct nsh_vtbl_s *vtbl);
static int   nsh_consoleoutput(struct nsh_vtbl_s *vtbl, const char *fmt, ...);
static int   nsh_erroroutput(struct nsh_vtbl_s *vtbl, const char *fmt, ...);
static char *nsh_consolelinebuffer(struct nsh_vtbl_s *vtbl);
static void  nsh_consoleexit(struct nsh_vtbl_s *vtbl, int exitstatus);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_consoleoutput
 *
 * Description:
 *   Print a string to the currently selected stream.
 *
 ****************************************************************************/

static int nsh_consoleoutput(struct nsh_vtbl_s *vtbl,
                             const char *fmt, ...)
{
    struct console_stdio_s *pstate = (struct console_stdio_s *)vtbl;
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vfprintf(stdout, fmt, ap);
    va_end(ap);

    return ret;
}

/****************************************************************************
 * Name: nsh_erroroutput
 *
 * Description:
 *   Print a string to the currently selected error stream.
 *
 ****************************************************************************/

static int nsh_erroroutput(struct nsh_vtbl_s *vtbl,
                           const char *fmt, ...)
{
    struct console_stdio_s *pstate = (struct console_stdio_s *)vtbl;
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vfprintf(stderr, fmt, ap);
    va_end(ap);

    return ret;
}

/****************************************************************************
 * Name: nsh_consolelinebuffer
 *
 * Description:
 *   Return a reference to the current line buffer
 *
 ****************************************************************************/

static char *nsh_consolelinebuffer(struct nsh_vtbl_s *vtbl)
{
    struct console_stdio_s *pstate = (struct console_stdio_s *)vtbl;
    return pstate->cn_line;
}

/****************************************************************************
 * Name: nsh_consoleclone
 *
 * Description:
 *   Make an independent copy of the vtbl
 *
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLEBG
static struct nsh_vtbl_s *nsh_consoleclone(struct nsh_vtbl_s *vtbl)
{
    struct console_stdio_s *pclone = nsh_newconsole();
    return &pclone->cn_vtbl;
}
#endif

/****************************************************************************
 * Name: nsh_consolerelease
 *
 * Description:
 *   Release the cloned instance
 *
 ****************************************************************************/

static void nsh_consolerelease(struct nsh_vtbl_s *vtbl)
{
    struct console_stdio_s *pstate = (struct console_stdio_s *)vtbl;

#ifdef CONFIG_NSH_VARS
    /* Free any NSH variables */

    if (pstate->varp != NULL) {
        free(pstate->varp);
    }
#endif

    /* Then release the vtable container */

    free(pstate);
}

/****************************************************************************
 * Name: nsh_consoleexit
 *
 * Description:
 *   Exit the shell task
 *
 ****************************************************************************/

static void nsh_consoleexit(struct nsh_vtbl_s *vtbl, int exitstatus)
{
    /* Destroy ourself then exit with the provided status */

    nsh_consolerelease(vtbl);
    exit(exitstatus);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct console_stdio_s *nsh_newconsole()
{
    struct console_stdio_s *pstate =
      (struct console_stdio_s *)malloc(sizeof(struct console_stdio_s));
    memset(pstate, 0, sizeof(struct console_stdio_s));

    if (pstate) {

        /* Initialize the call table */

#ifndef CONFIG_NSH_DISABLEBG
        pstate->cn_vtbl.clone       = nsh_consoleclone;
#endif
        pstate->cn_vtbl.release     = nsh_consolerelease;
        pstate->cn_vtbl.output      = nsh_consoleoutput;
        pstate->cn_vtbl.error       = nsh_erroroutput;
        pstate->cn_vtbl.linebuffer  = nsh_consolelinebuffer;
        pstate->cn_vtbl.exit        = nsh_consoleexit;

#ifndef CONFIG_NSH_DISABLESCRIPT
        /* Set the initial option flags */

        pstate->cn_vtbl.np.np_flags = NSH_NP_SET_OPTIONS_INIT;
#endif
    }

    return pstate;
}
