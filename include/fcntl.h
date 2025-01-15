#ifndef POSIX_FCNTL_H_
#define POSIX_FCNTL_H_

#include "sys/types.h"

#define O_RDONLY    (1 << 0)        /* Open for read access (only) */
#define O_RDOK      O_RDONLY        /* Read access is permitted (non-standard) */
#define O_WRONLY    (1 << 1)        /* Open for write access (only) */
#define O_WROK      O_WRONLY        /* Write access is permitted (non-standard) */
#define O_RDWR      (O_RDOK|O_WROK) /* Open for both read & write access */
#define O_CREAT     (1 << 2)        /* Create file/sem/mq object */
#define O_EXCL      (1 << 3)        /* Name must not exist when opened  */
#define O_APPEND    (1 << 4)        /* Keep contents, append to end */
#define O_TRUNC     (1 << 5)        /* Delete contents */
#define O_NONBLOCK  (1 << 6)        /* Don't wait for data */
#define O_NDELAY    O_NONBLOCK      /* Synonym for O_NONBLOCK */
#define O_SYNC      (1 << 7)        /* Synchronize output on write */
#define O_DSYNC     O_SYNC          /* Equivalent to OSYNC */
#define O_BINARY    (1 << 8)        /* Open the file in binary (untranslated) mode. */
#define O_DIRECT    (1 << 9)        /* Avoid caching, write directly to hardware */

#endif
