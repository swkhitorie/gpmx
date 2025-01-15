#ifndef POSIX_SYS_TYPES_H_
#define POSIX_SYS_TYPES_H_

#include <stdint.h>

#ifndef FAR
#define FAR
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#if !defined(__cplusplus)
#undef  ERROR
#define ERROR -1
#endif

typedef unsigned int mode_t;

typedef uintptr_t    size_t;
typedef intptr_t     ssize_t;
typedef uintptr_t    rsize_t;

typedef int16_t      pid_t;
typedef int32_t      off_t;

#if 1
typedef uint64_t     clock_t;
#else
typedef uint32_t     clock_t;
#endif

typedef uint32_t     useconds_t;
typedef int32_t      suseconds_t;

/* Task entry point */
typedef int (*main_t)(int argc, FAR char *argv[]);

#endif
