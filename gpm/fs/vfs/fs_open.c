#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"
#include "driver/driver.h"

static int file_vopen(struct file *filep, const char *path,
                      int oflags, mode_t umask, va_list ap)
{
    struct inode_search_s desc;
    struct inode *inode;
    mode_t mode = 0666;
    int ret;

    if (path == NULL) {
        return -EINVAL;
    }

    /* If the file is opened for creation, then get the mode bits */
    if ((oflags & (O_WRONLY | O_CREAT)) != 0) {
        mode = va_arg(ap, mode_t);
    }

    mode &= ~umask;

    /* Get an inode for this file */
    SETUP_SEARCH(&desc, path, false);

    ret = inode_find(&desc);
    if (ret < 0) {
        /* "O_CREAT is not set and the named file does not exist.  Or, a
        * directory component in pathname does not exist or is a dangling
        * symbolic link."
        */
        goto errout_with_search;
    }

    /* Get the search results */
    inode = desc.node;

    /* If the inode is block driver, then we may return a character driver
    * proxy for the block driver.  block_proxy() will instantiate a BCH
    * character driver wrapper around the block driver, open(), then
    * unlink() the character driver.
    *
    * NOTE: This will recurse to open the character driver proxy.
    */
    if (INODE_IS_BLOCK(inode) || INODE_IS_MTD(inode)) {

        inode_release(inode);
        RELEASE_SEARCH(&desc);
        return block_proxy(filep, path, oflags);
    }

    /* Make sure that the inode supports the requested access */
    ret = inode_checkflags(inode, oflags);
    if (ret < 0) {
        goto errout_with_inode;
    }

    filep->f_oflags = oflags;
    filep->f_pos    = 0;
    filep->f_inode  = inode;
    filep->f_priv   = NULL;

    /* Perform the driver open operation.  NOTE that the open method may be
    * called many times.  The driver/mountpoint logic should handled this
    * because it may also be closed that many times.
    */
    if (oflags & O_DIRECTORY) {

        ret = dir_allocate(filep, desc.relpath);
    } else if (INODE_IS_MOUNTPT(inode)) {

        if (inode->u.i_mops->open != NULL) {
            ret = inode->u.i_mops->open(filep, desc.relpath, oflags, mode);
        }
    } else if (INODE_IS_DRIVER(inode)) {

        if (inode->u.i_ops->open != NULL) {
            ret = inode->u.i_ops->open(filep);
        }
    } else {

        ret = -ENXIO;
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

    RELEASE_SEARCH(&desc);
    return 0; // OK

errout_with_inode:
    filep->f_inode = NULL;
    inode_release(inode);

errout_with_search:
    RELEASE_SEARCH(&desc);
    return ret;
}

/****************************************************************************
 * Name: nx_vopen
 ****************************************************************************/

static int nx_vopen(const char *path, int oflags, va_list ap)
{
    struct file filep;
    int ret;
    int fd;

    ret = file_vopen(&filep, path, oflags, getumask(), ap);
    if (ret < 0) {
        return ret;
    }

    /* Allocate a new file descriptor for the inode */
    fd = files_allocate(filep.f_inode, filep.f_oflags,
                        filep.f_pos, filep.f_priv, 0);
    if (fd < 0) {
        file_close(&filep);
        return fd;
    }

    return fd;
}

int inode_checkflags(struct inode *inode, int oflags)
{
    if (INODE_IS_PSEUDODIR(inode)) {
        return 0; // OK
    }

    if (inode->u.i_ops == NULL) {
        return -ENXIO;
    }

    if (((oflags & O_RDOK) != 0 && !inode->u.i_ops->read) ||
        ((oflags & O_WROK) != 0 && !inode->u.i_ops->write))
    {
        return -EACCES;
    } else {
        return 0; // OK
    }
}

int file_open(struct file *filep, const char *path, int oflags, ...)
{
    va_list ap;
    int ret;

    va_start(ap, oflags);
    ret = file_vopen(filep, path, oflags, 0, ap);
    va_end(ap);

    return ret;
}

int nx_open(const char *path, int oflags, ...)
{
    va_list ap;
    int fd;

    va_start(ap, oflags);
    fd = nx_vopen(path, oflags, ap);
    va_end(ap);

    return fd;
}

int open(const char *path, int oflags, ...)
{
    va_list ap;
    int fd;

    va_start(ap, oflags);
    fd = nx_vopen(path, oflags, ap);
    va_end(ap);

    if (fd < 0) {
        set_errno(-fd);
        fd = -1; // ERROR
    }

    return fd;
}
