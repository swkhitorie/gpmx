/****************************************************************************
 * apps/nshlib/nsh.h
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

#ifndef __APPS_NSHLIB_NSH_H
#define __APPS_NSHLIB_NSH_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <builtin/builtin.h>

#ifdef CONFIG_NSH_STRERROR
#include <string.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Verify support for ROMFS /etc directory support options */
#ifdef CONFIG_NSH_ROMFSETC
#ifdef CONFIG_DISABLE_MOUNTPOINT
#error "Mountpoint support is disabled"
#undef CONFIG_NSH_ROMFSETC
#endif

#ifndef CONFIG_FS_ROMFS
#error "ROMFS support not enabled"
#undef CONFIG_NSH_ROMFSETC
#endif

#ifndef CONFIG_NSH_ROMFSMOUNTPT
#define CONFIG_NSH_ROMFSMOUNTPT "/etc"
#endif

#ifndef CONFIG_NSH_SYSINITSCRIPT
#define CONFIG_NSH_SYSINITSCRIPT "init.d/rc.sysinit"
#endif

#ifndef CONFIG_NSH_INITSCRIPT
#define CONFIG_NSH_INITSCRIPT "init.d/rcS"
#endif

#undef NSH_SYSINITPATH
#define NSH_SYSINITPATH CONFIG_NSH_ROMFSMOUNTPT "/" CONFIG_NSH_SYSINITSCRIPT

#undef NSH_INITPATH
#define NSH_INITPATH CONFIG_NSH_ROMFSMOUNTPT "/" CONFIG_NSH_INITSCRIPT

#ifdef CONFIG_NSH_ROMFSRC
#ifndef CONFIG_NSH_RCSCRIPT
#define CONFIG_NSH_RCSCRIPT ".nshrc"
#endif

#undef NSH_RCPATH
#define NSH_RCPATH CONFIG_NSH_ROMFSMOUNTPT "/" CONFIG_NSH_RCSCRIPT
#endif

#ifndef CONFIG_NSH_ROMFSDEVNO
#define CONFIG_NSH_ROMFSDEVNO 0
#endif

#ifndef CONFIG_NSH_ROMFSSECTSIZE
#define CONFIG_NSH_ROMFSSECTSIZE 64
#endif

#define NSECTORS(b)        (((b)+CONFIG_NSH_ROMFSSECTSIZE-1)/CONFIG_NSH_ROMFSSECTSIZE)
#define STR_RAMDEVNO(m)    #m
#define MKMOUNT_DEVNAME(m) "/dev/ram" STR_RAMDEVNO(m)
#define MOUNT_DEVNAME      MKMOUNT_DEVNAME(CONFIG_NSH_ROMFSDEVNO)

#else

#undef CONFIG_NSH_ROMFSRC
#undef CONFIG_NSH_ROMFSMOUNTPT
#undef CONFIG_NSH_INITSCRIPT
#undef CONFIG_NSH_RCSCRIPT
#undef CONFIG_NSH_ROMFSDEVNO
#undef CONFIG_NSH_ROMFSSECTSIZE

#endif

/* Argument list size
 *
 *   argv[0]:      The command name.
 *   argv[1]:      The beginning of argument (up to CONFIG_NSH_MAXARGUMENTS)
 *   argv[argc-3]: Possibly '>' or '>>'
 *   argv[argc-2]: Possibly <file>
 *   argv[argc-1]: Possibly '&' (if pthreads are enabled)
 *   argv[argc]:   NULL terminating pointer
 *
 * Maximum size is CONFIG_NSH_MAXARGUMENTS+5
 */

#ifndef CONFIG_NSH_DISABLEBG
#define MAX_ARGV_ENTRIES (CONFIG_NSH_MAXARGUMENTS+5)
#else
#define MAX_ARGV_ENTRIES (CONFIG_NSH_MAXARGUMENTS+4)
#endif

/* strerror() produces much nicer output but is, however, quite large and
 * will only be used if CONFIG_NSH_STRERROR is defined.  Note that the
 * strerror interface must also have been enabled with CONFIG_LIBC_STRERROR.
 */

#ifndef CONFIG_LIBC_STRERROR
#undef CONFIG_NSH_STRERROR
#endif

#ifdef CONFIG_NSH_STRERROR
#define NSH_ERRNO          strerror(errno)
#define NSH_ERRNO_OF(err)  strerror(err)
#define NSH_HERRNO         gai_strerror(h_errno)
#define NSH_HERRNO_OF(err) gai_strerror(err)
#else
#define NSH_ERRNO          (errno)
#define NSH_ERRNO_OF(err)  (err)
#define NSH_HERRNO         (h_errno)
#define NSH_HERRNO_OF(err) (err)
#endif

/* Maximum size of one command line (telnet or serial) */

#ifndef CONFIG_NSH_LINELEN
#define CONFIG_NSH_LINELEN 80
#endif

/* The following two settings are used only in the telnetd interface */

#ifndef CONFIG_NSH_IOBUFFER_SIZE
#define CONFIG_NSH_IOBUFFER_SIZE 512
#endif

/* The maximum number of nested if-then[-else]-fi sequences that
 * are permissible.
 */

#ifndef CONFIG_NSH_NESTDEPTH
#define CONFIG_NSH_NESTDEPTH 3
#endif

#undef NSH_HAVE_VARS
#if defined(CONFIG_NSH_VARS) || !defined(CONFIG_DISABLE_ENVIRON)
# define NSH_HAVE_VARS
#endif

/* The size of the I/O buffer may be specified in the
 * boards/<arch>/<chip>/<board>/configs/<config>defconfig file -- provided
 * that it is at least as large as PATH_MAX.
 */

#define NSH_HAVE_IOBUFFER 1

/* The I/O buffer is needed for the ls, cp, and ps commands.  It is also
 * needed if the platform supplied MOTD is configured.
 */

#if defined(CONFIG_NSH_DISABLE_LS) && defined(CONFIG_NSH_DISABLE_CP) && \
    defined(CONFIG_NSH_DISABLE_PS) && !defined(CONFIG_NSH_PLATFORM_MOTD) && \
    defined(CONFIG_DISABLE_ENVIRON)
#undef NSH_HAVE_IOBUFFER
#endif

#ifdef NSH_HAVE_IOBUFFER
#ifdef CONFIG_NSH_FILEIOSIZE
#if CONFIG_NSH_FILEIOSIZE > (PATH_MAX + 1)
#define IOBUFFERSIZE CONFIG_NSH_FILEIOSIZE
#else
#define IOBUFFERSIZE (PATH_MAX + 1)
#endif
#else
#define IOBUFFERSIZE 1024
#endif
#else
#define IOBUFFERSIZE (PATH_MAX + 1)
#endif


#if !defined(CONFIG_NSH_VARS) || defined(CONFIG_DISABLE_ENVIRON)
#undef CONFIG_NSH_DISABLE_EXPORT
#define CONFIG_NSH_DISABLE_EXPORT 1
#endif

#ifndef CONFIG_NSH_DISABLESCRIPT
#define NSH_NP_SET_OPTIONS "ex"    /* Maintain order see nsh_npflags_e */
#define NSH_NP_SET_OPTIONS_INIT    (NSH_PFLAG_SILENT)
#endif

#if !defined(NSH_HAVE_VARS) && defined(CONFIG_NSH_DISABLESCRIPT)
#undef  CONFIG_NSH_DISABLE_SET
#define CONFIG_NSH_DISABLE_SET 1
#undef  CONFIG_NSH_DISABLE_UNSET
#define CONFIG_NSH_DISABLE_UNSET 1
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_ITEF
/* State when parsing and if-then-else sequence */

enum nsh_itef_e {
    NSH_ITEF_NORMAL = 0,         /* Not in an if-then-else sequence */
    NSH_ITEF_IF,                 /* Just parsed 'if', expect condition */
    NSH_ITEF_THEN,               /* Just parsed 'then', looking for 'else' or 'fi' */
    NSH_ITEF_ELSE                /* Just parsed 'else', look for 'fi' */
};

/* All state data for parsing one if-then-else sequence */

struct nsh_itef_s {
    uint8_t   ie_ifcond   : 1;   /* Value of command in 'if' statement */
    uint8_t   ie_disabled : 1;   /* TRUE: Unconditionally disabled */
    uint8_t   ie_inverted : 1;   /* TRUE: inverted logic ('if ! <cmd>') */
    uint8_t   ie_unused   : 3;
    uint8_t   ie_state    : 2;   /* If-then-else state (see enum nsh_itef_e) */
};
#endif

#ifndef CONFIG_NSH_DISABLE_LOOPS
/* State when parsing and while-do-done or until-do-done sequence */

enum nsh_lp_e {
    NSH_LOOP_NORMAL = 0,         /* Not in a while-do-done or until-do-done sequence */
    NSH_LOOP_WHILE,              /* Just parsed 'while', expect condition */
    NSH_LOOP_UNTIL,              /* Just parsed 'until', expect condition */
    NSH_LOOP_DO                  /* Just parsed 'do', looking for 'done' */
};

/* All state data for parsing one while-do-done or until-do-done sequence */

struct nsh_loop_s {
    uint8_t   lp_enable   : 1;   /* Loop command processing is enabled */
    uint8_t   lp_unused   : 5;
    uint8_t   lp_state    : 2;   /* Loop state (see enume nsh_lp_e) */
#ifndef CONFIG_NSH_DISABLE_ITEF
    uint8_t   lp_iendx;          /* Saved if-then-else-fi index */
#endif
    long      lp_topoffs;        /* Top of loop file offset */
};
#endif

#ifndef CONFIG_NSH_DISABLESCRIPT
/* Define the bits that correspond to the option defined in
 * NSH_NP_SET_OPTIONS. The bit value is 1 shifted left the offset
 * of the char in NSH_NP_SET_OPTIONS string.
 */

enum nsh_npflags_e {
    NSH_PFLAG_IGNORE = 1,      /*  set for +e no exit on errors,
                                *  cleared -e exit on error */
    NSH_PFLAG_SILENT = 2,      /*  cleared -x  print a trace of commands
                                *  when parsing.
                                *  set +x no print a trace of commands */
};
#endif

struct nsh_script_streams {
    int    np_stream;
};

/* These structure provides the overall state of the parser */

struct nsh_parser_s {
#ifndef CONFIG_NSH_DISABLEBG
    bool     np_bg;       /* true: The last command executed in background */
#endif
#ifdef CONFIG_FILE_STREAM
    bool     np_redirect; /* true: Output from the last command was re-directed */
#endif
    bool     np_fail;     /* true: The last command failed */
#ifndef CONFIG_NSH_DISABLESCRIPT
    uint8_t  np_flags;    /* See nsh_npflags_e above */
#endif
#ifndef CONFIG_NSH_DISABLEBG
    int      np_nice;     /* "nice" value applied to last background cmd */
#endif

#ifndef CONFIG_NSH_DISABLESCRIPT
    int      np_fd;       /* fd of current script */
#ifndef CONFIG_NSH_DISABLE_LOOPS
    long     np_foffs;    /* File offset to the beginning of a line */
#ifndef NSH_DISABLE_SEMICOLON
    uint16_t np_loffs;    /* Byte offset to the beginning of a command */
    bool     np_jump;     /* "Jump" to the top of the loop */
#endif
    uint8_t  np_lpndx;    /* Current index into np_lpstate[] */
#endif
#ifndef CONFIG_NSH_DISABLE_ITEF
    uint8_t  np_iendx;    /* Current index into np_iestate[] */
#endif

    /* This is a stack of parser state information. */

#ifndef CONFIG_NSH_DISABLE_ITEF
    struct nsh_itef_s np_iestate[CONFIG_NSH_NESTDEPTH];
#endif
#ifndef CONFIG_NSH_DISABLE_LOOPS
    struct nsh_loop_s np_lpstate[CONFIG_NSH_NESTDEPTH];
#endif
#endif
};

/* This is the general form of a command handler */

struct nsh_vtbl_s; /* Defined in nsh_console.h */
typedef int (*nsh_cmd_t)(struct nsh_vtbl_s *vtbl, int argc,
                         char **argv);

#if defined(CONFIG_NSH_VARS) && !defined(CONFIG_NSH_DISABLE_SET)
/* Used with nsh_foreach_var() */

typedef int (*nsh_foreach_var_t)(struct nsh_vtbl_s *vtbl, void *arg,
                                 const char *pair);
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

extern const char g_nshgreeting[];
extern const char g_nshprompt[];
extern const char g_fmtsyntax[];
extern const char g_fmtargrequired[];
extern const char g_fmtnomatching[];
extern const char g_fmtarginvalid[];
extern const char g_fmtargrange[];
extern const char g_fmtcmdnotfound[];
extern const char g_fmtnosuch[];
extern const char g_fmttoomanyargs[];
extern const char g_fmtdeepnesting[];
extern const char g_fmtcontext[];
extern const char g_fmtcmdfailed[];
extern const char g_fmtcmdoutofmemory[];
extern const char g_fmtinternalerror[];
extern const char g_fmtsignalrecvd[];

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/* Initialization */

#ifdef CONFIG_NSH_ROMFSETC
int nsh_romfsetc(void);
#else
#define nsh_romfsetc() (-ENOSYS)
#endif

#ifdef HAVE_USB_CONSOLE
int nsh_usbconsole(void);
#else
#define nsh_usbconsole() (-ENOSYS)
#endif

#if !defined(CONFIG_NSH_DISABLESCRIPT)
int nsh_script(struct nsh_vtbl_s *vtbl, const char *cmd,
               const char *path);
int nsh_initscript(struct nsh_vtbl_s *vtbl);
#endif

/* Basic session and message handling */

struct console_stdio_s;
int nsh_session(struct console_stdio_s *pstate,
                bool login, int argc, char *argv[]);
int nsh_parse(struct nsh_vtbl_s *vtbl, char *cmdline);

/* Application interface */

int nsh_command(struct nsh_vtbl_s *vtbl, int argc, char *argv[]);

#ifdef CONFIG_NSH_BUILTIN_APPS
int nsh_builtin(struct nsh_vtbl_s *vtbl, const char *cmd, char **argv);
#endif

/* Debug */
void nsh_dumpbuffer(struct nsh_vtbl_s *vtbl, const char *msg,
                    const uint8_t *buffer, ssize_t nbytes);


#if !defined(CONFIG_NSH_DISABLESCRIPT) && !defined(CONFIG_NSH_DISABLE_TEST)
int cmd_test(struct nsh_vtbl_s *vtbl, int argc, char **argv);
int cmd_lbracket(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#ifndef CONFIG_NSH_DISABLE_SET
int cmd_set(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#ifndef CONFIG_NSH_DISABLE_UNSET
int cmd_unset(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#ifdef CONFIG_NSH_VARS
char *nsh_getvar(struct nsh_vtbl_s *vtbl, const char *name);
#ifndef CONFIG_NSH_DISABLE_SET
int   nsh_setvar(struct nsh_vtbl_s *vtbl, const char *name, const char *value);
#endif
#if !defined(CONFIG_NSH_DISABLE_UNSET) || !defined(CONFIG_NSH_DISABLE_EXPORT)
int   nsh_unsetvar(struct nsh_vtbl_s *vtbl, const char *name);
#endif
#endif

#if !defined(CONFIG_NSH_DISABLESCRIPT) && !defined(CONFIG_NSH_DISABLE_LOOPS)
int cmd_break(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#ifndef CONFIG_NSH_DISABLE_ECHO
int cmd_echo(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#ifndef CONFIG_NSH_DISABLE_EXEC
int cmd_exec(struct nsh_vtbl_s *vtbl, int argc, char **argv);
#endif

#if defined(CONFIG_NSH_VARS) && !defined(CONFIG_NSH_DISABLE_SET)
int nsh_foreach_var(struct nsh_vtbl_s *vtbl, nsh_foreach_var_t cb,
                    void *arg);
#endif

int nsh_consolemain(int argc, char *argv[]);

#if defined(__cplusplus)
}
#endif

#endif /* __APPS_NSHLIB_NSH_H */
