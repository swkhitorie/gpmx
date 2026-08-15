/****************************************************************************
 * include/stdio.h
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

#ifndef __INCLUDE_STDIO_H
#define __INCLUDE_STDIO_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <sys/types.h>
#include <stdarg.h>
#include <time.h>

#include <gpm/fs/fs.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* File System Definitions **************************************************/

#define FILENAME_MAX 16

/* The (default) size of the I/O buffers */

#if defined(CONFIG_STDIO_BUFFER_SIZE) && CONFIG_STDIO_BUFFER_SIZE > 0
#define BUFSIZ   CONFIG_STDIO_BUFFER_SIZE
#else
#define BUFSIZ   64
#endif

/* The following three definitions are for ANSI C, used by setvbuf */

#define _IOFBF     0               /* Fully buffered */
#define _IOLBF     1               /* Line buffered */
#define _IONBF     2               /* Unbuffered */

/* File system error values */

#define EOF        (-1)

/* The first three _iob entries are reserved for standard I/O */

#define stdin      (&nxsched_get_streams()->sl_std[0])
#define stdout     (&nxsched_get_streams()->sl_std[1])
#define stderr     (&nxsched_get_streams()->sl_std[2])

/* Path to the directory where temporary files can be created */

#ifndef CONFIG_LIBC_TMPDIR
#define CONFIG_LIBC_TMPDIR "/tmp"
#endif

#define P_tmpdir   CONFIG_LIBC_TMPDIR

/* Maximum size of character array to hold tmpnam() output. */

#ifndef CONFIG_LIBC_MAX_TMPFILE
#define CONFIG_LIBC_MAX_TMPFILE 32
#endif

#define L_tmpnam   CONFIG_LIBC_MAX_TMPFILE

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/* Streams */

typedef struct file_struct FILE;

struct va_format
{
    const char *fmt;
    va_list *va;
};

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* ANSI-like File System Interfaces */

/* Operations on streams (FILE) */

void   clearerr(FILE *stream);
int    fclose(FILE *stream);
int    fflush(FILE *stream);
int    feof(FILE *stream);
int    ferror(FILE *stream);
int    fileno(FILE *stream);
int    fgetc(FILE *stream);
int    fgetpos(FILE *stream, fpos_t *pos);
char  *fgets(char *s, int n, FILE *stream);
FILE  *fopen(const char *path, const char *type);
int    fprintf(FILE *stream, const char *format, ...);
int    fputc(int c, FILE *stream);
int    fputs(const char *s, FILE *stream);
size_t fread(void *ptr, size_t size, size_t n_items, FILE *stream);
int    fseek(FILE *stream, long int offset, int whence);
long   ftell(FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t n_items,
         FILE *stream);
int     getc(FILE *stream);
int     getchar(void);
char   *gets(char *s);
char   *gets_s(char *s, rsize_t n);

void   setbuf(FILE *stream, char *buf);
int    setvbuf(FILE *stream, char *buffer, int mode, size_t size);

int    ungetc(int c, FILE *stream);

/* Operations on the stdout stream, buffers, paths,
 * and the whole printf-family
 */

void   perror(const char *s);
int    printf(const char *fmt, ...);
int    putc(int c, FILE *stream);
int    putchar(int c);
int    puts(const char *s);
int    rename(const char *oldpath, const char *newpath);
int    sprintf(char *buf, const char *fmt, ...);
int    asprintf(char **ptr, const char *fmt, ...);
int    snprintf(char *buf, size_t size,
         const char *fmt, ...);

int    vasprintf(char **ptr, const char *fmt, va_list ap);
int    vfprintf(FILE *stream, const char *fmt,
        va_list ap);
int    vfscanf(FILE *stream, const char *fmt, va_list ap);
int    vprintf(const char *fmt, va_list ap);
int    vsnprintf(char *buf, size_t size, const char *fmt,
        va_list ap);
int    vsprintf(char *buf, const char *fmt, va_list ap);

/* Operations on file descriptors including:
 *
 * POSIX-like File System Interfaces (fdopen), and
 * Extensions from the Open Group Technical Standard, 2006, Extended API Set
 *   Part 1 (dprintf and vdprintf)
 */

FILE *fdopen(int fd, const char *type);
int   dprintf(int fd, const char *fmt, ...);
int   vdprintf(int fd, const char *fmt, va_list ap);


#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_STDIO_H */
