#ifndef POSIX_SYS_TYPES_H_
#define POSIX_SYS_TYPES_H_

#include <stdint.h>

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned int mode_t;

typedef uintptr_t    size_t;
typedef intptr_t     ssize_t;
typedef uintptr_t    rsize_t;

/* uid_t is used for user IDs
 * gid_t is used for group IDs.
 */

typedef int16_t      uid_t;
typedef int16_t      gid_t;

/* dev_t is used for device IDs */

typedef uint32_t     dev_t;

/* ino_t is used for file serial numbers */

typedef uint16_t     ino_t;

/* nlink_t is used for link counts */

typedef uint16_t     nlink_t;

/* pid_t is used for process IDs and process group IDs. It must be signed
 * because negative PID values are used to represent invalid PIDs.
 */

typedef int          pid_t;

/* id_t is a general identifier that can be used to contain at least a pid_t,
 * uid_t, or gid_t.
 */

typedef int          id_t;

/* Unix requires a key of type key_t defined in file sys/types.h for
 * requesting resources such as shared memory segments, message queues and
 * semaphores. A key is simply an integer of type key_t
 */

typedef int32_t      key_t;

/* Signed integral type of the result of subtracting two pointers */

typedef intptr_t     ptrdiff_t;

/* wint_t
 *   An integral type capable of storing any valid value of wchar_t, or WEOF.
 */

// typedef int wint_t;

/* wctype_t
 *   A scalar type of a data object that can hold values which represent
 *   locale-specific character classification.
 */

typedef int wctype_t;

/* fsblkcnt_t and fsfilcnt_t shall be defined as unsigned integer types. */

typedef uint32_t     fsblkcnt_t;
typedef uint32_t     fsfilcnt_t;

/* blkcnt_t and off_t are signed integer types.
 *
 *   blkcnt_t is used for file block counts.
 *   off_t is used for file offsets and sizes.
 *   fpos_t is used for file positions.
 *
 * Hence, all should be independent of processor architecture.
 */

typedef uint32_t     blkcnt_t;
typedef int32_t      off_t;
typedef int32_t      fpos_t;

/* blksize_t is a signed integer value used for file block sizes */

typedef int16_t      blksize_t;

typedef uint32_t     blkcnt_t;
typedef int16_t      blksize_t;

typedef int32_t      off_t;

#if 0
typedef uint64_t     clock_t;
#else
typedef uint32_t     clock_t;
#endif

/* The type useconds_t shall be an unsigned integer type capable of storing
 * values at least in the range [0, 1000000]. The type suseconds_t shall be
 * a signed integer type capable of storing values at least in the range
 * [-1, 1000000].
 */

typedef uint32_t     useconds_t;
typedef int32_t      suseconds_t;

/* These were defined by ISO C without the first `_'.  */

typedef uint8_t  u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;

/* Task entry point */
typedef int (*main_t)(int argc, char *argv[]);

/* POSIX-like OS return values: */

// enum {
//     ERROR = -1,
//     OK = 0,
// };

#undef  NAME_MAX
#ifndef CONFIG_NAME_MAX
#define NAME_MAX 40
#else
#define NAME_MAX CONFIG_NAME_MAX
#endif

#undef  PATH_MAX
#ifndef CONFIG_PATH_MAX
#define PATH_MAX 256
#else
#define PATH_MAX CONFIG_PATH_MAX
#endif

#undef  OPEN_MAX
#ifndef CONFIG_LIBC_OPEN_MAX
#define OPEN_MAX 255
#elif
#define OPEN_MAX CONFIG_LIBC_OPEN_MAX
#endif

#endif
