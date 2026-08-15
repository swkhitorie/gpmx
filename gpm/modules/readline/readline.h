/****************************************************************************
 * apps/system/readline/readline.h
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

#ifndef __APPS_SYSTEM_READLINE_READLINE_H
#define __APPS_SYSTEM_READLINE_READLINE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Tab completion cannot be supported if there is no console echo */
#ifndef CONFIG_READLINE_ECHO
#undef CONFIG_READLINE_TABCOMPLETION
#endif

/* Make sure that the are valid values for all tab-completion settings */
#ifdef CONFIG_READLINE_TABCOMPLETION
#ifndef CONFIG_READLINE_MAX_BUILTINS
#define CONFIG_READLINE_MAX_BUILTINS 64
#endif

#ifndef CONFIG_READLINE_MAX_EXTCMDS
#define CONFIG_READLINE_MAX_EXTCMDS 64
#endif
#endif

/* Some environments may return CR as end-of-line, others LF, and others
 * both.  If not specified, the logic here assumes either (but not both) as
 * the default.
 */

#if defined(CONFIG_EOL_IS_CR)
#undef  CONFIG_EOL_IS_LF
#undef  CONFIG_EOL_IS_BOTH_CRLF
#undef  CONFIG_EOL_IS_EITHER_CRLF
#elif defined(CONFIG_EOL_IS_LF)
#undef  CONFIG_EOL_IS_CR
#undef  CONFIG_EOL_IS_BOTH_CRLF
#undef  CONFIG_EOL_IS_EITHER_CRLF
#elif defined(CONFIG_EOL_IS_BOTH_CRLF)
#undef  CONFIG_EOL_IS_CR
#undef  CONFIG_EOL_IS_LF
#undef  CONFIG_EOL_IS_EITHER_CRLF
#elif defined(CONFIG_EOL_IS_EITHER_CRLF)
#undef  CONFIG_EOL_IS_CR
#undef  CONFIG_EOL_IS_LF
#undef  CONFIG_EOL_IS_BOTH_CRLF
#else
#undef  CONFIG_EOL_IS_CR
#undef  CONFIG_EOL_IS_LF
#undef  CONFIG_EOL_IS_BOTH_CRLF
#define CONFIG_EOL_IS_EITHER_CRLF 1
#endif

/* Helper macros */

#define RL_GETC(v)      ((v)->rl_getc(v))

#ifdef CONFIG_READLINE_ECHO
#define RL_PUTC(v,ch)   ((v)->rl_putc(v,ch))
#define RL_WRITE(v,b,s) ((v)->rl_write(v,b,s))
#endif

/****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

struct rl_common_s
{
  int  (*rl_getc)(struct rl_common_s *vtbl);
#ifdef CONFIG_READLINE_ECHO
  void (*rl_putc)(struct rl_common_s *vtbl, int ch);
  void (*rl_write)(struct rl_common_s *vtbl, const char *buffer,
                  size_t buflen);
#endif
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_READLINE_TABCOMPLETION
const char *readline_prompt(const char *prompt);
#else
#define readline_prompt(p)
#endif

#define readline(b,s,ifd,ofd) readline_fd(b,s,ifd,ofd)

ssize_t readline_fd(char *buf, int buflen, int infd, int outfd);

ssize_t readline_common(struct rl_common_s *vtbl, char *buf, int buflen);

#endif /* __APPS_SYSTEM_READLINE_READLINE_H */
