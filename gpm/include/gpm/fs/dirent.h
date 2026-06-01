#ifndef INCLUDE_FS_DIRENT_H_
#define INCLUDE_FS_DIRENT_H_

#include <sys/types.h>
#include <stdint.h>
#include <dirent.h>

#include "gpm/fs/fs.h"

struct fs_pseudodir_s {
    struct inode *fd_next;             /* The inode for the next call to readdir() */
};

struct fs_dirent_s
{
  /* This is the node that was opened by opendir.  The type of the inode
   * determines the way that the readdir() operations are performed. For the
   * pseudo root pseudo-file system, it is also used to support rewind.
   *
   * We hold a reference on this inode so we know that it will persist until
   * closedir() is called (although inodes linked to this inode may change).
   */

  struct inode *fd_root;

  /* At present, only mountpoints require special handling flags */
  unsigned int fd_flags;

  /* This keeps track of the current directory position for telldir */

  off_t fd_position;

  /* Retained control information depends on the type of file system that
   * provides the mountpoint.  Ideally this information should
   * be hidden behind an opaque, file-system-dependent void *, but we put
   * the private definitions in line here for now to reduce allocations.
   */

  union
    {
      /* Private data used by the built-in pseudo-file system */

      struct fs_pseudodir_s pseudo;
  } u;

  /* In any event, this the actual struct dirent that is returned by
   * readdir
   */

  struct dirent fd_dir;              /* Populated when readdir is called */
};

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif
