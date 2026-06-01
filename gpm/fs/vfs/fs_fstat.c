#include <sys/stat.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "gpm/fs/fs.h"
#include "gpm/mtd/mtd.h"
#include "inode/inode.h"

/****************************************************************************
 * Name: proxy_fstat
 *
 * Description:
 *   Check for special cases where the character driver is really just a
 *   proxy for the real, underlying MTD or block driver.
 *
 *   NOTE:  This must be done here rather than in the the common
 *   inode_stat() function because the filep reference must be available
 *   in order to call the character driver ioctl method.
 *
 * Input Parameters:
 *   filep  - File structure instance
 *   inode  - The inode associated with the file descriptor
 *   buf    - The caller provide location in which to return information
 *            about the open file.
 *
 * Returned Value:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 *
 ****************************************************************************/
static int proxy_fstat(struct file *filep, struct inode *inode,
                       struct stat *buf)
{
    struct mtd_geometry_s mtdgeo;
    struct geometry blkgeo;
    int ret = -ENOENT;

    if (INODE_IS_DRIVER(inode) &&
        inode->u.i_ops != NULL &&
        inode->u.i_ops->ioctl != NULL)
    {

        /* Check if this is a proxy for an MTD driver.  In this case, both the
        * MTDIOC_GEOMETRY ioctl and the BIOC_GEOMTRY will be supported by
        * character driver.
        */
        if (inode->u.i_ops->ioctl(filep, MTDIOC_GEOMETRY,
                                    (unsigned long)((uintptr_t)&mtdgeo)) >= 0)
        {
            memset(buf, 0, sizeof(struct stat));
            buf->st_mode  = S_IFMTD;
            buf->st_mode |= S_IROTH | S_IRGRP | S_IRUSR;
            buf->st_mode |= S_IWOTH | S_IWGRP | S_IWUSR;
            buf->st_size  = mtdgeo.neraseblocks * mtdgeo.erasesize;
            ret           = 0;

        /* Check if this is a proxy for a block driver.  In this case, only
        * the BIOC_GEOMETRY ioctl will be supported.
        */
        } else if (inode->u.i_ops->ioctl(filep, BIOC_GEOMETRY,
                (unsigned long)((uintptr_t)&blkgeo)) >= 0)
        {
            memset(buf, 0, sizeof(struct stat));
            buf->st_mode = S_IFBLK;
            if (inode->u.i_ops->read) {
                buf->st_mode |= S_IROTH | S_IRGRP | S_IRUSR;
            }

            if (inode->u.i_ops->write) {
                buf->st_mode |= S_IWOTH | S_IWGRP | S_IWUSR;
            }

            if (blkgeo.geo_available) {
                buf->st_size = blkgeo.geo_nsectors * blkgeo.geo_sectorsize;
            }
            ret = 0;
        }
    }

    return ret;
}

/****************************************************************************
 * Name: file_fstat
 *
 * Description:
 *   file_fstat() is an internal OS interface.  It is functionally similar
 *   to the standard fstat() interface except:
 *
 *    - It does not modify the errno variable,
 *    - It is not a cancellation point,
 *    - It does not handle socket descriptors, and
 *    - It accepts a file structure instance instead of file descriptor.
 *
 * Input Parameters:
 *   filep  - File structure instance
 *   buf    - The caller provide location in which to return information
 *            about the open file.
 *
 * Returned Value:
 *   Upon successful completion, 0 shall be returned. Otherwise, -1 shall be
 *   returned and errno set to indicate the error.
 *
 ****************************************************************************/
int file_fstat(struct file *filep, struct stat *buf)
{
    struct inode *inode;
    int ret;

    inode = filep->f_inode;
    if (!inode) {
        return -EBADF;
    }

    if (INODE_IS_MOUNTPT(inode)) {
        /* The inode is a file system mountpoint. Verify that the mountpoint
        * supports the fstat() method
        */
        ret = -ENOSYS;
        if (inode->u.i_mops && inode->u.i_mops->fstat) {
            /* Perform the fstat() operation */
            ret = inode->u.i_mops->fstat(filep, buf);
        }
    } else {
        /* Check if the inode is a proxy for a block or MTD driver */
        ret = proxy_fstat(filep, inode, buf);
        if (ret < 0) {
            /* The inode is part of the root pseudo file system. */
            ret = inode_stat(inode, buf, 0);
        }
    }

    return ret;
}

int fstat(int fd, struct stat *buf)
{
    struct file *filep;
    int ret;

    ret = fs_getfilep(fd, &filep);
    if (ret < 0) {
        goto errout;
    }

    ret = file_fstat(filep, buf);
    if (ret >= 0) {
        return 0;
    }

errout:
    set_errno(-ret);
    return -1;
}
