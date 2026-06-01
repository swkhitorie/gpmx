#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include "gpm/fs/fs.h"
#include "gpm/fs/ioctl.h"

#include "inode/inode.h"

/****************************************************************************
 * Name: file_fsync
 *
 * Description:
 *   Equivalent to the standard fsync() function except that is accepts a
 *   struct file instance instead of a file descriptor and it does not set
 *   the errno variable.
 *
 ****************************************************************************/
int file_fsync(struct file *filep)
{
    struct inode *inode;
    int ret;

    /* Is this inode a registered mountpoint? Does it support the
    * sync operations may be relevant to device drivers but only
    * the mountpoint operations vtable contains a sync method.
    */
    inode = filep->f_inode;
    if (inode != NULL) {
        if (INODE_IS_MOUNTPT(inode) && inode->u.i_mops &&
            inode->u.i_mops->sync)
        {
            /* Yes, then tell the mountpoint to sync this file */
            return inode->u.i_mops->sync(filep);
        } else if (inode->u.i_ops && inode->u.i_ops->ioctl) {
            ret = inode->u.i_ops->ioctl(filep, BIOC_FLUSH, 0);
            return ret >= 0 ? 0 : ret;
        }
    }

    return -EINVAL;
}

int fsync(int fd)
{
    struct file *filep;
    int ret;

    ret = fs_getfilep(fd, &filep);
    if (ret < 0) {
        goto errout;
    }

    ret = file_fsync(filep);
    if (ret < 0) {
        goto errout;
    }

    return ret;

errout:
    set_errno(-ret);
    return -1;
}
