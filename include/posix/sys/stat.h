#ifndef POSIX_SYS_STAT_H_
#define POSIX_SYS_STAT_H_

#include <sys/types.h>
#include <time.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/

#define S_IXOTH     (1 << 0)   /* Bits 0-2: Permissions for others: RWX */
#define S_IWOTH     (1 << 1)
#define S_IROTH     (1 << 2)
#define S_IRWXO     (7 << 0)

#define S_IXGRP     (1 << 3)   /* Bits 3-5: Group permissions: RWX */
#define S_IWGRP     (1 << 4)
#define S_IRGRP     (1 << 5)
#define S_IRWXG     (7 << 3)

#define S_IXUSR     (1 << 6)   /* Bits 6-8: Owner permissions: RWX */
#define S_IWUSR     (1 << 7)
#define S_IRUSR     (1 << 8)
#define S_IRWXU     (7 << 6)

#define S_ISVTX     0          /* "Sticky" bit (not used) */
#define S_ISGID     0          /* Set group ID bit (not used)*/
#define S_ISUID     0          /* Set UID bit (not used) */

#define S_IFIFO     0          /* Bits 11-14: File type bits (not all used) */
#define S_IFCHR     (1 << 11)
#define S_IFDIR     (2 << 11)
#define S_IFBLK     (3 << 11)
#define S_IFREG     (4 << 11)
#define S_IFMQ      (5 << 11)
#define S_IFSEM     (6 << 11)
#define S_IFSHM     (7 << 11)
#define S_IFSOCK    (8 << 11)
#define S_IFMTD     (9 << 11)
#define s_IFTGT     (15 << 11) /* May be the target of a symbolic link */

#define S_IFLNK     (1 << 15)  /* Bit 15: Symbolic link */
#define S_IFMT      (31 << 11) /* Bits 11-15: Full file type */

struct stat
{
  /* Required, standard fields */

  mode_t    st_mode;    /* File type, attributes, and access mode bits */
  off_t     st_size;    /* Size of file/directory, in bytes */
  blksize_t st_blksize; /* Block size used for filesystem I/O */
  blkcnt_t  st_blocks;  /* Number of blocks allocated */
  time_t    st_atime;   /* Time of last access */
  time_t    st_mtime;   /* Time of last modification */
  time_t    st_ctime;   /* Time of last status change */

};

#endif
