/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include <gpm/fs/fs.h>
#include <driver/drv_sched.h>
#include <gpm/mtd/mtd.h>
#include <semaphore.h>

#include <sys/stat.h>
#include <sys/statfs.h>

#include "ff.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct fatfs_dir_s
{
    struct fs_dirent_s    base;
    FDIR                  dir;
};

struct fatfs_file_s
{
    FIL                   file;
    int                   refs;
};

/* This structure represents the overall mountpoint state. An instance of
 * this structure is retained as inode private data on each mountpoint that
 * is mounted with a fatfs filesystem.
 */

struct fatfs_mountpt_s
{
    sem_t                 sem;
    FATFS                 fs;
    BYTE                  pdrv;
    char                  path[4];
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void    fatfs_semgive(struct fatfs_mountpt_s *fs);
static int     fatfs_semtake(struct fatfs_mountpt_s *fs);

static int     fatfs_open(struct file *filep, const char *relpath,
                            int oflags, mode_t mode);
static int     fatfs_close(struct file *filep);
static ssize_t fatfs_read(struct file *filep, char *buffer,
                            size_t buflen);
static ssize_t fatfs_write(struct file *filep, const char *buffer,
                            size_t buflen);
static off_t   fatfs_seek(struct file *filep, off_t offset,
                            int whence);
static int     fatfs_ioctl(struct file *filep, int cmd,
                            unsigned long arg);

static int     fatfs_sync(struct file *filep);
static int     fatfs_dup(const struct file *oldp,
                            struct file *newp);
static int     fatfs_fstat(const struct file *filep,
                              struct stat *buf);
static int     fatfs_truncate(struct file *filep,
                                off_t length);

static int     fatfs_opendir(struct inode *mountpt,
                                const char *relpath,
                                struct fs_dirent_s **dir);
static int     fatfs_closedir(struct inode *mountpt,
                                 struct fs_dirent_s *dir);
static int     fatfs_readdir(struct inode *mountpt,
                                struct fs_dirent_s *dir,
                                struct dirent *entry);
static int     fatfs_rewinddir(struct inode *mountpt,
                                  struct fs_dirent_s *dir);

static int     fatfs_bind(struct inode *driver,
                            const void *data, void **handle);
static int     fatfs_unbind(void *handle, struct inode **driver,
                            unsigned int flags);
static int     fatfs_statfs(struct inode *mountpt,
                            struct statfs *buf);

static int     fatfs_unlink(struct inode *mountpt,
                            const char *relpath);
static int     fatfs_mkdir(struct inode *mountpt,
                            const char *relpath, mode_t mode);
static int     fatfs_rmdir(struct inode *mountpt,
                            const char *relpath);
static int     fatfs_rename(struct inode *mountpt,
                            const char *oldrelpath,
                            const char *newrelpath);
static int     fatfs_stat(struct inode *mountpt,
                            const char *relpath, struct stat *buf);

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* See fs_mount.c -- this structure is explicitly extern'ed there.
 * We use the old-fashioned kind of initializers so that this will compile
 * with any compiler.
 */

const struct mountpt_operations fatfs_operations =
{
    fatfs_open,          /* open */
    fatfs_close,         /* close */
    fatfs_read,          /* read */
    fatfs_write,         /* write */
    fatfs_seek,          /* seek */
    fatfs_ioctl,         /* ioctl */

    fatfs_sync,          /* sync */
    fatfs_dup,           /* dup */
    fatfs_fstat,         /* fstat */
    NULL,                /* fchstat */
    fatfs_truncate,      /* truncate */

    fatfs_opendir,       /* opendir */
    fatfs_closedir,      /* closedir */
    fatfs_readdir,       /* readdir */
    fatfs_rewinddir,     /* rewinddir */

    fatfs_bind,          /* bind */
    fatfs_unbind,        /* unbind */
    fatfs_statfs,        /* statfs */

    fatfs_unlink,        /* unlink */
    fatfs_mkdir,         /* mkdir */
    fatfs_rmdir,         /* rmdir */
    fatfs_rename,        /* rename */
    fatfs_stat,          /* stat */
    NULL                 /* chstat */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: fatfs_semtake
 ****************************************************************************/

static int fatfs_semtake(struct fatfs_mountpt_s *fs)
{
    return sem_wait_uninterruptible(&fs->sem);
}

/****************************************************************************
 * Name: fatfs_semgive
 ****************************************************************************/

static void fatfs_semgive(struct fatfs_mountpt_s *fs)
{
    sem_post(&fs->sem);
}

/****************************************************************************
 * Name: fatfs_convert_result
 ****************************************************************************/
static int fatfs_convert_result(int ret)
{
    int status = 0;

    switch (ret) {
    case FR_OK:
        break;

    case FR_NO_FILE:
    case FR_NO_PATH:
    case FR_NO_FILESYSTEM:
        status = -ENOENT;
        break;

    case FR_INVALID_NAME:
    case FR_INVALID_PARAMETER:
    case FR_INVALID_DRIVE:
        status = -EINVAL;
        break;

    case FR_EXIST:
    case FR_INVALID_OBJECT:
        status = -EEXIST;
        break;

    case FR_DISK_ERR:
    case FR_NOT_READY:
    case FR_INT_ERR:
    case FR_NOT_ENABLED:
        status = -EIO;
        break;

    case FR_WRITE_PROTECTED:
    case FR_DENIED:
        status = -EROFS;
        break;

    case FR_MKFS_ABORTED:
        status = -EINVAL;
        break;

    case FR_TOO_MANY_OPEN_FILES:
        status = -EMFILE;

    default:
        status = -1;
        break;
    }

    return status;
}

/****************************************************************************
 * Name: fatfs_convert_oflags
 ****************************************************************************/

static int fatfs_convert_oflags(int oflags)
{
    int ret = 0;

    switch (oflags & (O_RDONLY|O_WRONLY|O_RDWR)) {

    case O_RDONLY:
        ret |= FA_READ;
        break;
    case O_WRONLY:
        ret |= FA_WRITE;
        break;
    case O_RDWR:
        ret |= FA_READ | FA_WRITE;
        break;
    default:
        ret |= FA_READ;
    }

    if ((oflags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {

        /** create new file, failed if file already exist */
        ret |= FA_CREATE_NEW;
    } else if ((oflags & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) {

        /** always create new file */
        ret |= FA_CREATE_ALWAYS;
    } else if (oflags & O_CREAT) {

        /** create file when it not exist, open file when it existing */
        ret |= FA_OPEN_ALWAYS;
    } else if (oflags & O_TRUNC) {

        ret |= FA_OPEN_EXISTING;
    } else {

        ret |= FA_OPEN_EXISTING;
    }

    return ret;
}

/****************************************************************************
 * Name: fatfs_open
 ****************************************************************************/

static int fatfs_open(struct file *filep, const char *relpath,
                        int oflags, mode_t mode)
{
    struct fatfs_mountpt_s *fs;
    char volpath[64];
    struct fatfs_file_s *priv;
    struct inode *inode;
    BYTE fat_oflags;
    int ret;

    /* Get the mountpoint inode reference from the file structure and the
    * mountpoint private data from the inode structure
    */

    inode = filep->f_inode;
    fs    = inode->i_private;

    /* Allocate memory for the open file */

    priv = kmm_malloc(sizeof(*priv));
    if (priv == NULL) {
        return -ENOMEM;
    }

    priv->refs = 1;

    /* Take the semaphore */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        goto errsem;
    }

    /* Try to open the file */

    snprintf(volpath, 64, "%s%s", fs->path, relpath);
    fat_oflags = fatfs_convert_oflags(oflags);
    ret = fatfs_convert_result(f_open(&priv->file, volpath, 
                                fat_oflags));

    if (ret < 0) {
        /* Error opening file */

        goto errout;
    }

    if ((oflags & O_TRUNC) && !(oflags & O_CREAT)) {

        ret = fatfs_convert_result(f_truncate(&priv->file));

        if (ret < 0) {

            goto errout_with_file;
        }
    }

    /* In append mode, we need to set the file pointer to the end of the
    * file.
    */

    if (oflags & O_APPEND) {
        ret = fatfs_convert_result(f_lseek(&priv->file, 
                                    f_size(&priv->file)));
        if (ret >= 0) {

            filep->f_pos = f_tell(&priv->file);
        } else {

            goto errout_with_file;
        }
    }

    fatfs_semgive(fs);

    /* Attach the private date to the struct file instance */

    filep->f_priv = priv;

    return 0;

errout_with_file:
    f_close(&priv->file);
errout:
    fatfs_semgive(fs);
errsem:
    kmm_free(priv);
    return ret;
}

/****************************************************************************
 * Name: fatfs_close
 ****************************************************************************/

static int fatfs_close(struct file *filep)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    int ret;

    /* Recover our private data from the struct file instance */

    priv  = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    /* Close the file */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    if (--priv->refs <= 0) {
        ret = fatfs_convert_result(f_close(&priv->file));
    }

    fatfs_semgive(fs);
    if (priv->refs <= 0) {
        kmm_free(priv);
    }

    return ret;
}

/****************************************************************************
 * Name: fatfs_read
 ****************************************************************************/

static ssize_t fatfs_read(struct file *filep, char *buffer,
                            size_t buflen)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    unsigned int br;
    ssize_t ret;

    /* Recover our private data from the struct file instance */

    priv  = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    /* Call LFS to perform the read */

    ret = fatfs_semtake(fs);
    if (ret < 0) {

        return ret;
    }

    if (filep->f_pos != f_tell(&priv->file)) {
        ret = fatfs_convert_result(f_lseek(&priv->file,
                                        filep->f_pos));
        if (ret < 0) {
            goto out;
        }
    }

    ret = fatfs_convert_result(f_read(&priv->file,
                buffer, buflen, &br));
    if (ret == FR_OK) {

        filep->f_pos += br;
        ret = br;
    }

out:
    fatfs_semgive(fs);
    return ret;
}

/****************************************************************************
 * Name: fatfs_write
 ****************************************************************************/

static ssize_t fatfs_write(struct file *filep, const char *buffer,
                            size_t buflen)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    unsigned int bw;
    ssize_t ret;

    /* Recover our private data from the struct file instance */

    priv  = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    /* Call LFS to perform the write */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    if (filep->f_pos != f_tell(&priv->file)) {
        ret = fatfs_convert_result(f_lseek(&priv->file,
                                        filep->f_pos));
        if (ret < 0) {
            goto out;
        }
    }

    ret = fatfs_convert_result(f_write(&priv->file,
                                        buffer, buflen, &bw));
    if (ret == FR_OK) {

        filep->f_pos += bw;
        ret = bw;
    }

out:
    fatfs_semgive(fs);
    return ret;
}

/****************************************************************************
 * Name: fatfs_seek
 ****************************************************************************/

static off_t fatfs_seek(struct file *filep, off_t offset, int whence)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    off_t ret;

    /* Recover our private data from the struct file instance */

    priv  = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    /* Call LFS to perform the seek */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    ret = fatfs_convert_result(f_lseek(&priv->file, offset));

    if (ret >= 0) {

        filep->f_pos = f_tell(&priv->file);
        ret = filep->f_pos;
    }

    fatfs_semgive(fs);
    return ret;
}

/****************************************************************************
 * Name: fatfs_ioctl
 ****************************************************************************/

static int fatfs_ioctl(struct file *filep, int cmd, unsigned long arg)
{
    return -ENOTTY;
}

/****************************************************************************
 * Name: fatfs_sync
 *
 * Description: Synchronize the file state on disk to match internal, in-
 *   memory state.
 *
 ****************************************************************************/

static int fatfs_sync(struct file *filep)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    int ret;

    /* Recover our private data from the struct file instance */

    priv  = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    ret = fatfs_convert_result(f_sync(&priv->file));
    fatfs_semgive(fs);

    return ret;
}

/****************************************************************************
 * Name: fatfs_dup
 *
 * Description: Duplicate open file data in the new file structure.
 *
 ****************************************************************************/

static int fatfs_dup(const struct file *oldp, struct file *newp)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    int ret;

    /* Recover our private data from the struct file instance */

    priv  = oldp->f_priv;
    inode = oldp->f_inode;
    fs    = inode->i_private;

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    priv->refs++;
    newp->f_priv = priv;
    fatfs_semgive(fs);

    return ret;
}

/****************************************************************************
 * Name: fatfs_fstat
 *
 * Description:
 *   Obtain information about an open file associated with the file
 *   descriptor 'fd', and will write it to the area pointed to by 'buf'.
 *
 ****************************************************************************/

static int fatfs_fstat(const struct file *filep, struct stat *buf)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    int ret;

    memset(buf, 0, sizeof(*buf));

    /* Recover our private data from the struct file instance */

    priv  = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    /* Call LFS to get file size */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    buf->st_size = f_size(&priv->file);
    fatfs_semgive(fs);

    buf->st_mode    = S_IRWXO | S_IRWXG | S_IRWXU | S_IFREG;
    buf->st_blksize = 512;
    buf->st_blocks  = (buf->st_size + 511) / 512;

    return 0;
}

/****************************************************************************
 * Name: fatfs_truncate
 *
 * Description:
 *   Set the length of the open, regular file associated with the file
 *   structure 'filep' to 'length'.
 *
 ****************************************************************************/

static int fatfs_truncate(struct file *filep, off_t length)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_file_s *priv;
    struct inode *inode;
    int ret;

    /* Recover our private data from the struct file instance */

    priv  = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    /* Call LFS to perform the truncate */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    ret = fatfs_convert_result(f_lseek(&priv->file, length));
    if (ret == 0) {
        ret = fatfs_convert_result(f_truncate(&priv->file));
    }

    fatfs_semgive(fs);

    return ret;
}

/****************************************************************************
 * Name: fatfs_opendir
 *
 * Description: Open a directory for read access
 *
 ****************************************************************************/

static int fatfs_opendir(struct inode *mountpt,
                        const char *relpath,
                        struct fs_dirent_s **dir)
{
    struct fatfs_mountpt_s *fs;
    char volpath[64];
    struct fatfs_dir_s *fdir;
    int ret;

    /* Recover our private data from the inode instance */

    fs = mountpt->i_private;

    /* Allocate memory for the open directory */

    fdir = kmm_malloc(sizeof(*fdir));
    if (fdir == NULL) {
        return -ENOMEM;
    }

    /* Take the semaphore */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        goto errsem;
    }

    /* Call the Fatfs's opendir function */
    snprintf(volpath, 64, "%s%s", fs->path, relpath);
    ret = fatfs_convert_result(f_opendir(&fdir->dir, volpath));
    if (ret < 0) {
        goto errout;
    }

    fatfs_semgive(fs);
    *dir = &fdir->base;
    return 0;

errout:
    fatfs_semgive(fs);
errsem:
    kmm_free(fdir);
    return ret;
}

/****************************************************************************
 * Name: fatfs_closedir
 *
 * Description: Close a directory
 *
 ****************************************************************************/

static int fatfs_closedir(struct inode *mountpt,
                             struct fs_dirent_s *dir)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_dir_s *fdir;
    int ret;

    /* Recover our private data from the inode instance */

    fdir = (struct fatfs_dir_s *)dir;
    fs   = mountpt->i_private;

    /* Call the LFS's closedir function */

    ret = fatfs_semtake(fs);
    if (ret < 0) {

        return ret;
    }

    f_closedir(&fdir->dir);
    fatfs_semgive(fs);

    kmm_free(fdir);
    return 0;
}

/****************************************************************************
 * Name: fatfs_readdir
 *
 * Description: Read the next directory entry
 *
 ****************************************************************************/

static int fatfs_readdir(struct inode *mountpt,
                            struct fs_dirent_s *dir,
                            struct dirent *entry)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_dir_s *fdir;
    FILINFO info;
    int ret;

    /* Recover our private data from the inode instance */

    fdir = (struct fatfs_dir_s *)dir;
    fs   = mountpt->i_private;

    /* Call the Fatfs's readdir function */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    ret = fatfs_convert_result(f_readdir(&fdir->dir, &info));
    if (ret >= 0) {

        if (info.fname[0] == '\0') {

            ret = -ENOENT;
        } else {

            entry->d_type = (info.fattrib & AM_DIR) ? DTYPE_DIRECTORY : DTYPE_FILE;
            strlcpy(entry->d_name, info.fname, sizeof(entry->d_name));
            ret = 0;
        }
    }

    fatfs_semgive(fs);
    return ret;
}

/****************************************************************************
 * Name: fatfs_rewindir
 *
 * Description: Reset directory read to the first entry
 *
 ****************************************************************************/

static int fatfs_rewinddir(struct inode *mountpt,
                              struct fs_dirent_s *dir)
{
    struct fatfs_mountpt_s *fs;
    struct fatfs_dir_s *fdir;
    int ret;

    /* Recover our private data from the inode instance */

    fdir = (struct fatfs_dir_s *)dir;
    fs   = mountpt->i_private;

    /* Call the Fatfs's rewinddir function */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    f_closedir(&fdir->dir);
    ret = fatfs_convert_result(f_opendir(&fdir->dir, "."));

    fatfs_semgive(fs);
    return ret;
}

/****************************************************************************
 * Name: fatfs_bind
 *
 * Description: This implements a portion of the mount operation. This
 *  function allocates and initializes the mountpoint private data and
 *  binds the driver inode to the filesystem private data. The final
 *  binding of the private data (containing the driver) to the
 *  mountpoint is performed by mount().
 *
 ****************************************************************************/

static int fatfs_bind(struct inode *driver, const void *data,
                         void **handle)
{
    struct fatfs_mountpt_s *fs;
    const char *options = data;
    int ret;

    fs = kmm_zalloc(sizeof(*fs));
    if (!fs) {
        return -ENOMEM;
    }

    sem_init(&fs->sem, 0, 0);

    if (options && options[0] != '\0') {

        strncpy(fs->path, options, sizeof(fs->path) - 1);
        fs->path[sizeof(fs->path) - 1] = '\0';
    } else {

        strcpy(fs->path, "0:");
    }

    ret = fatfs_convert_result(f_mount(&fs->fs, fs->path, 1));

    if (ret < 0) {

        if (options && strcmp(options, "forceformat") == 0) {

            MKFS_PARM mkfs_parm = { FM_FAT | FM_FAT32, 1, 0, 0, 0 };

            ret = fatfs_convert_result(f_mkfs(fs->path, &mkfs_parm, NULL, 0));
            if (ret == 0) {

                ret = fatfs_convert_result(f_mount(&fs->fs, fs->path, 1));
            }
        }

        if (ret < 0) {
            goto errout_with_fs;
        }
    }

    *handle = fs;

    fatfs_semgive(fs);
    return 0;

errout_with_fs:
    sem_destroy(&fs->sem);
    kmm_free(fs);
    return ret;
}

/****************************************************************************
 * Name: fatfs_unbind
 *
 * Description: This implements the filesystem portion of the umount
 *  operation.
 *
 ****************************************************************************/

static int fatfs_unbind(void *handle, struct inode **driver,
                        unsigned int flags)
{
    struct fatfs_mountpt_s *fs = handle;
    int ret;

    /* Unmount */

    ret = fatfs_semtake(fs);
    if (ret < 0) {

        return ret;
    }

    ret = fatfs_convert_result(f_mount(NULL, fs->path, 0));
    fatfs_semgive(fs);

    if (ret >= 0) {

        sem_destroy(&fs->sem);
        kmm_free(fs);
    }

    return ret;
}

/****************************************************************************
 * Name: fatfs_statfs
 *
 * Description: Return filesystem statistics
 *
 ****************************************************************************/

static int fatfs_statfs(struct inode *mountpt, struct statfs *buf)
{
    struct fatfs_mountpt_s *fs;
    DWORD free_clst;
    FATFS *fs_ptr;
    int ret;

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Return something for the file system description */

    memset(buf, 0, sizeof(*buf));
    buf->f_type    = FATFS_SUPER_MAGIC;
    buf->f_namelen = 255;

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    ret = fatfs_convert_result(f_getfree(fs->path, &free_clst, &fs_ptr));
    if (ret >= 0) {
        buf->f_bsize   = fs_ptr->ssize;
        DWORD total_clusters = fs_ptr->n_fatent - 2;
        buf->f_blocks  = total_clusters * fs_ptr->csize;
        buf->f_bfree   = free_clst * fs_ptr->csize;
        buf->f_bavail  = buf->f_bfree;
    }

    fatfs_semgive(fs);
    return ret;
}

/****************************************************************************
 * Name: fatfs_unlink
 *
 * Description: Remove a file
 *
 ****************************************************************************/

static int fatfs_unlink(struct inode *mountpt,
                           const char *relpath)
{
    struct fatfs_mountpt_s *fs;
    char volpath[64];
    int ret;

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Call the Fatfs to perform the unlink */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    snprintf(volpath, 64, "%s%s", fs->path, relpath);
    ret = fatfs_convert_result(f_unlink(volpath));
    fatfs_semgive(fs);

    return ret;
}

/****************************************************************************
 * Name: fatfs_mkdir
 *
 * Description: Create a directory
 *
 ****************************************************************************/

static int fatfs_mkdir(struct inode *mountpt, const char *relpath,
                        mode_t mode)
{
    struct fatfs_mountpt_s *fs;
    char volpath[64];
    int ret;

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Call Fatfs to do the mkdir */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    snprintf(volpath, 64, "%s%s", fs->path, relpath);
    ret = fatfs_convert_result(f_mkdir(volpath));
    fatfs_semgive(fs);

    return ret;
}

/****************************************************************************
 * Name: fatfs_rmdir
 *
 * Description: Remove a directory
 *
 ****************************************************************************/

static int fatfs_rmdir(struct inode *mountpt, const char *relpath)
{
    return fatfs_unlink(mountpt, relpath);
}

/****************************************************************************
 * Name: fatfs_rename
 *
 * Description: Rename a file or directory
 *
 ****************************************************************************/

static int fatfs_rename(struct inode *mountpt,
                           const char *oldrelpath,
                           const char *newrelpath)
{
    struct fatfs_mountpt_s *fs;
    char volpath_old[64];
    char volpath_new[64];
    int ret;

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Call Fatfs to do the rename */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    snprintf(volpath_old, 64, "%s%s", fs->path, oldrelpath);
    snprintf(volpath_new, 64, "%s%s", fs->path, newrelpath);
    ret = fatfs_convert_result(f_rename(volpath_old,
                                volpath_new));
    fatfs_semgive(fs);

    return ret;
}

/****************************************************************************
 * Name: fatfs_stat
 *
 * Description: Return information about a file or directory
 *
 ****************************************************************************/

static int fatfs_stat(struct inode *mountpt, const char *relpath,
                         struct stat *buf)
{
    struct fatfs_mountpt_s *fs;
    char volpath[64];
    FILINFO info;
    int ret;

    memset(buf, 0, sizeof(*buf));

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Call the LFS to do the stat operation */

    ret = fatfs_semtake(fs);
    if (ret < 0) {
        return ret;
    }

    snprintf(volpath, 64, "%s%s", fs->path, relpath);
    ret = fatfs_convert_result(f_stat(volpath, &info));
    fatfs_semgive(fs);

    if (ret >= 0) {
        /* Convert info to stat */

        buf->st_mode = S_IRWXO | S_IRWXG | S_IRWXU;
        if (info.fattrib & AM_DIR) {

            buf->st_mode |= S_IFDIR;
        } else {

            buf->st_mode |= S_IFREG;
            buf->st_size = info.fsize;
        }
        buf->st_blksize = 512;
        buf->st_blocks  = (buf->st_size + 511) / 512;
    }

    return ret;
}
