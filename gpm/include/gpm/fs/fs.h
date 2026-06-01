#ifndef __INCLUDE_FS_H
#define __INCLUDE_FS_H

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <dirent.h>

#include "gpm/mutex.h"

#ifndef CONFIG_NFILE_DESCRIPTORS_PER_BLOCK
#define CONFIG_NFILE_DESCRIPTORS_PER_BLOCK 8
#endif

/* Stream flags for the fs_flags field of in struct file_struct */
#define __FS_FLAG_EOF   (1 << 0) /* EOF detected by a read operation */
#define __FS_FLAG_ERROR (1 << 1) /* Error detected by any operation */
#define __FS_FLAG_LBF   (1 << 2) /* Line buffered */
#define __FS_FLAG_UBF   (1 << 3) /* Buffer allocated by caller of setvbuf */

/* Inode i_flags values:
 *
 *   Bit 0-3: Inode type (Bit 3 indicates internal OS types)
 *   Bit 4:   Set if inode has been unlinked and is pending removal.
 */

#define FSNODEFLAG_TYPE_MASK        0x0000000f /* Isolates type field      */
#define   FSNODEFLAG_TYPE_PSEUDODIR 0x00000000 /*   Pseudo dir (default)   */
#define   FSNODEFLAG_TYPE_DRIVER    0x00000001 /*   Character driver       */
#define   FSNODEFLAG_TYPE_BLOCK     0x00000002 /*   Block driver           */
#define   FSNODEFLAG_TYPE_MOUNTPT   0x00000003 /*   Mount point            */
#define   FSNODEFLAG_TYPE_NAMEDSEM  0x00000004 /*   Named semaphore        */
#define   FSNODEFLAG_TYPE_MQUEUE    0x00000005 /*   Message Queue          */
#define   FSNODEFLAG_TYPE_SHM       0x00000006 /*   Shared memory region   */
#define   FSNODEFLAG_TYPE_MTD       0x00000007 /*   Named MTD driver       */
#define   FSNODEFLAG_TYPE_SOFTLINK  0x00000008 /*   Soft link              */
#define   FSNODEFLAG_TYPE_SOCKET    0x00000009 /*   Socket                 */
#define FSNODEFLAG_DELETED          0x00000010 /* Unlinked                 */

#define INODE_IS_TYPE(i,t) \
  (((i)->i_flags & FSNODEFLAG_TYPE_MASK) == (t))

#define INODE_IS_PSEUDODIR(i) INODE_IS_TYPE(i,FSNODEFLAG_TYPE_PSEUDODIR)
#define INODE_IS_DRIVER(i)    INODE_IS_TYPE(i,FSNODEFLAG_TYPE_DRIVER)
#define INODE_IS_BLOCK(i)     INODE_IS_TYPE(i,FSNODEFLAG_TYPE_BLOCK)
#define INODE_IS_MOUNTPT(i)   INODE_IS_TYPE(i,FSNODEFLAG_TYPE_MOUNTPT)
#define INODE_IS_NAMEDSEM(i)  INODE_IS_TYPE(i,FSNODEFLAG_TYPE_NAMEDSEM)
#define INODE_IS_MQUEUE(i)    INODE_IS_TYPE(i,FSNODEFLAG_TYPE_MQUEUE)
#define INODE_IS_SHM(i)       INODE_IS_TYPE(i,FSNODEFLAG_TYPE_SHM)
#define INODE_IS_MTD(i)       INODE_IS_TYPE(i,FSNODEFLAG_TYPE_MTD)
#define INODE_IS_SOFTLINK(i)  INODE_IS_TYPE(i,FSNODEFLAG_TYPE_SOFTLINK)
#define INODE_IS_SOCKET(i)    INODE_IS_TYPE(i,FSNODEFLAG_TYPE_SOCKET)

#define INODE_GET_TYPE(i)     ((i)->i_flags & FSNODEFLAG_TYPE_MASK)
#define INODE_SET_TYPE(i,t) \
  do \
    { \
      (i)->i_flags = ((i)->i_flags & ~FSNODEFLAG_TYPE_MASK) | (t); \
    } \
  while (0)

#define INODE_SET_DRIVER(i)   INODE_SET_TYPE(i,FSNODEFLAG_TYPE_DRIVER)
#define INODE_SET_BLOCK(i)    INODE_SET_TYPE(i,FSNODEFLAG_TYPE_BLOCK)
#define INODE_SET_MOUNTPT(i)  INODE_SET_TYPE(i,FSNODEFLAG_TYPE_MOUNTPT)
#define INODE_SET_NAMEDSEM(i) INODE_SET_TYPE(i,FSNODEFLAG_TYPE_NAMEDSEM)
#define INODE_SET_MQUEUE(i)   INODE_SET_TYPE(i,FSNODEFLAG_TYPE_MQUEUE)
#define INODE_SET_SHM(i)      INODE_SET_TYPE(i,FSNODEFLAG_TYPE_SHM)
#define INODE_SET_MTD(i)      INODE_SET_TYPE(i,FSNODEFLAG_TYPE_MTD)
#define INODE_SET_SOFTLINK(i) INODE_SET_TYPE(i,FSNODEFLAG_TYPE_SOFTLINK)
#define INODE_SET_SOCKET(i)   INODE_SET_TYPE(i,FSNODEFLAG_TYPE_SOCKET)

/* The status change flags.
 * These should be or-ed together to figure out what want to change.
 */

#define CH_STAT_MODE       (1 << 0)
#define CH_STAT_UID        (1 << 1)
#define CH_STAT_GID        (1 << 2)
#define CH_STAT_ATIME      (1 << 3)
#define CH_STAT_MTIME      (1 << 4)

/* nx_umount() is equivalent to nx_umount2() with flags = 0 */
#define umount(t)       umount2(t,0)


/* Forward references */
struct file;
struct inode;
struct stat;
struct statfs;
struct pollfd;
struct mtd_dev_s;

/* The internal representation of type DIR is just a container for an inode
 * reference, and the path of directory.
 */
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

  /* The path name of current directory for FIOC_FILEPATH */
  char *fd_path;
};

/* This structure is provided by devices when they are registered with the
 * system.  It is used to call back to perform device specific operations.
 */

struct file_operations
{
  /* The device driver open method differs from the mountpoint open method */
  int     (*open)(struct file *filep);

  /* The following methods must be identical in signature and position
   * because the struct file_operations and struct mountp_operations are
   * treated like unions.
   */
  int     (*close)(struct file *filep);
  ssize_t (*read)(struct file *filep, char *buffer, size_t buflen);
  ssize_t (*write)(struct file *filep, const char *buffer,
                   size_t buflen);
  off_t   (*seek)(struct file *filep, off_t offset, int whence);
  int     (*ioctl)(struct file *filep, int cmd, unsigned long arg);

  /* The two structures need not be common after this point */
  int     (*poll)(struct file *filep, struct pollfd *fds, bool setup);
  int     (*unlink)(struct inode *inode);
};

/* This structure provides information about the state of a block driver */

struct geometry
{
  bool      geo_available;    /* true: The device is available */
  bool      geo_mediachanged; /* true: The media has changed since last query */
  bool      geo_writeenabled; /* true: It is okay to write to this device */
  blkcnt_t  geo_nsectors;     /* Number of sectors on the device */
  blksize_t geo_sectorsize;   /* Size of one sector */
};

struct partition_info_s
{
  size_t    numsectors;   /* Number of sectors in the partition */
  size_t    sectorsize;   /* Size in bytes of a single sector */
  off_t     startsector;  /* Offset to the first section/block of the
                           * managed sub-region */

  /* NULL-terminated string representing the name of the parent node of the
   * partition.
   */

  char      parent[NAME_MAX + 1];
};

/* This structure is provided by block devices when they register with the
 * system.  It is used by file systems to perform filesystem transfers.  It
 * differs from the normal driver vtable in several ways -- most notably in
 * that it deals in struct inode vs. struct filep.
 */

struct inode;
struct block_operations
{
  int     (*open)(struct inode *inode);
  int     (*close)(struct inode *inode);
  ssize_t (*read)(struct inode *inode, unsigned char *buffer,
            blkcnt_t start_sector, unsigned int nsectors);
  ssize_t (*write)(struct inode *inode, const unsigned char *buffer,
            blkcnt_t start_sector, unsigned int nsectors);
  int     (*geometry)(struct inode *inode, struct geometry
                      *geometry);
  int     (*ioctl)(struct inode *inode, int cmd, unsigned long arg);
  int     (*unlink)(struct inode *inode);
};

/* This structure is provided by a filesystem to describe a mount point.
 * Note that this structure differs from file_operations ONLY in the form of
 * the open method.  Once the file is opened, it can be accessed either as a
 * struct file_operations or struct mountpt_operations
 */

struct mountpt_operations
{
  /* The mountpoint open method differs from the driver open method
   * because it receives (1) the inode that contains the mountpoint
   * private data, (2) the relative path into the mountpoint, and (3)
   * information to manage privileges.
   */

  int     (*open)(struct file *filep, const char *relpath,
            int oflags, mode_t mode);

  /* The following methods must be identical in signature and position
   * because the struct file_operations and struct mountpt_operations are
   * treated like unions.
   */

  int     (*close)(struct file *filep);
  ssize_t (*read)(struct file *filep, char *buffer, size_t buflen);
  ssize_t (*write)(struct file *filep, const char *buffer,
            size_t buflen);
  off_t   (*seek)(struct file *filep, off_t offset, int whence);
  int     (*ioctl)(struct file *filep, int cmd, unsigned long arg);

  /* The two structures need not be common after this point. The following
   * are extended methods needed to deal with the unique needs of mounted
   * file systems.
   *
   * Additional open-file-specific mountpoint operations:
   */

  int     (*sync)(struct file *filep);
  int     (*dup)(const struct file *oldp, struct file *newp);
  int     (*fstat)(const struct file *filep, struct stat *buf);
  int     (*fchstat)(const struct file *filep,
                     const struct stat *buf, int flags);
  int     (*truncate)(struct file *filep, off_t length);

  /* Directory operations */

  int     (*opendir)(struct inode *mountpt, const char *relpath,
            struct fs_dirent_s **dir);
  int     (*closedir)(struct inode *mountpt,
            struct fs_dirent_s *dir);
  int     (*readdir)(struct inode *mountpt,
            struct fs_dirent_s *dir, struct dirent *entry);
  int     (*rewinddir)(struct inode *mountpt,
            struct fs_dirent_s *dir);

  /* General volume-related mountpoint operations: */

  int     (*bind)(struct inode *blkdriver, const void *data,
            void **handle);
  int     (*unbind)(void *handle, struct inode **blkdriver,
            unsigned int flags);
  int     (*statfs)(struct inode *mountpt, struct statfs *buf);

  /* Operations on paths */

  int     (*unlink)(struct inode *mountpt, const char *relpath);
  int     (*mkdir)(struct inode *mountpt, const char *relpath,
            mode_t mode);
  int     (*rmdir)(struct inode *mountpt, const char *relpath);
  int     (*rename)(struct inode *mountpt, const char *oldrelpath,
            const char *newrelpath);
  int     (*stat)(struct inode *mountpt, const char *relpath,
            struct stat *buf);
  int     (*chstat)(struct inode *mountpt, const char *relpath,
            const struct stat *buf, int flags);
};

/* These are the various kinds of operations that can be associated with
 * an inode.
 */
union inode_ops_u
{
  const struct file_operations     *i_ops;    /* Driver operations for inode */
  const struct block_operations    *i_bops;   /* Block driver operations */
  struct mtd_dev_s                 *i_mtd;    /* MTD device driver */
  const struct mountpt_operations  *i_mops;   /* Operations on a mountpoint */
};

struct inode
{
  struct inode *i_parent;   /* Link to parent level inode */
  struct inode *i_peer;     /* Link to same level inode */
  struct inode *i_child;    /* Link to lower level inode */
  int16_t           i_crefs;    /* References to inode */
  uint16_t          i_flags;    /* Flags for inode */
  union inode_ops_u u;          /* Inode operations */
  void         *i_private;  /* Per inode driver private data */
  char          i_name[1];  /* Name of inode (variable) */
};

#define FSNODE_SIZE(n) (sizeof(struct inode) + (n))

/* This is the underlying representation of an open file.  A file
 * descriptor is an index into an array of such types. The type associates
 * the file descriptor to the file state and to a set of inode operations.
 */

struct file
{
  int               f_oflags;   /* Open mode flags */
  off_t             f_pos;      /* File position */
  struct inode *f_inode;    /* Driver or file system interface */
  void         *f_priv;     /* Per file driver private data */
};

/* This defines a two layer array of files indexed by the file descriptor.
 * Each row of this array is fixed size: CONFIG_NFILE_DESCRIPTORS_PER_BLOCK.
 * You can get file instance in filelist by the follow methods:
 * (file descriptor / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK) as row index and
 * (file descriptor % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK) as column index.
 */

struct filelist
{
  sem_t             fl_sem;     /* Manage access to the file list */
  uint8_t           fl_rows;    /* The number of rows of fl_files array */
  struct file **fl_files;   /* The pointer of two layer file descriptors array */
};

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

void fs_initialize(void);

int register_driver(const char *path,
                    const struct file_operations *fops, mode_t mode,
                    void *priv);

int register_blockdriver(const char *path,
                         const struct block_operations *bops,
                         mode_t mode, void *priv);

int register_blockpartition(const char *partition,
                            mode_t mode, const char *parent,
                            off_t firstsector, off_t nsectors);

int unregister_driver(const char *path);

int unregister_blockdriver(const char *path);

int register_mtddriver(const char *path, struct mtd_dev_s *mtd,
                       mode_t mode, void *priv);

int register_mtdpartition(const char *partition,
                          mode_t mode, const char *parent,
                          off_t firstblock, off_t nblocks);

int unregister_mtddriver(const char *path);

int nx_mount(const char *source, const char *target,
             const char *filesystemtype, unsigned long mountflags,
             const void *data);

int nx_umount2(const char *target, unsigned int flags);

void files_initlist(struct filelist *list);

void files_releaselist(struct filelist *list);

int files_duplist(struct filelist *plist, struct filelist *clist);

int file_dup(struct file *filep, int minfd);

int nx_dup(int fd);

int file_dup2(struct file *filep1, struct file *filep2);

int nx_dup2(int fd1, int fd2);

int file_open(struct file *filep, const char *path, int oflags, ...);

int nx_open(const char *path, int oflags, ...);

int fs_getfilep(int fd, struct file **filep);

int file_close(struct file *filep);

int nx_close(int fd);

int open_blockdriver(const char *pathname, int mountflags,
                     struct inode **ppinode);

int close_blockdriver(struct inode *inode);

ssize_t file_read(struct file *filep, void *buf, size_t nbytes);

ssize_t nx_read(int fd, void *buf, size_t nbytes);

ssize_t file_write(struct file *filep, const void *buf,
                   size_t nbytes);

ssize_t nx_write(int fd, const void *buf, size_t nbytes);

ssize_t file_pread(struct file *filep, void *buf, size_t nbytes,
                   off_t offset);

ssize_t file_pwrite(struct file *filep, const void *buf,
                    size_t nbytes, off_t offset);

ssize_t file_sendfile(struct file *outfile, struct file *infile,
                      off_t *offset, size_t count);

off_t file_seek(struct file *filep, off_t offset, int whence);

off_t nx_seek(int fd, off_t offset, int whence);

int file_fsync(struct file *filep);

int file_truncate(struct file *filep, off_t length);

int file_ioctl(struct file *filep, int req, ...);

int nx_ioctl(int fd, int req, ...);

int file_fcntl(struct file *filep, int cmd, ...);

int nx_fcntl(int fd, int cmd, ...);

int file_poll(struct file *filep, struct pollfd *fds, bool setup);

int nx_poll(struct pollfd *fds, unsigned int nfds, int timeout);

int file_fstat(struct file *filep, struct stat *buf);

int nx_stat(const char *path, struct stat *buf, int resolve);

int file_fchstat(struct file *filep, struct stat *buf, int flags);

int nx_unlink(const char *pathname);

#if defined(CONFIG_PIPES) && CONFIG_DEV_PIPE_SIZE > 0
int file_pipe(struct file *filep[2], size_t bufsize, int flags);
int nx_pipe(int fd[2], size_t bufsize, int flags);
#endif

#if defined(CONFIG_PIPES) && CONFIG_DEV_FIFO_SIZE > 0
int nx_mkfifo(const char *pathname, mode_t mode, size_t bufsize);
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_NUTTX_FS_FS_H */
