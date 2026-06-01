#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <stdarg.h>

#include "inode/inode.h"

int file_vioctl(struct file *filep, int req, va_list ap)
{
    struct inode *inode;
    unsigned long arg;
    int ret = -ENOTTY;

    arg = va_arg(ap, unsigned long);

    inode = filep->f_inode;
    if (!inode) {
        return -EBADF;
    }

    /* Does the driver support the ioctl method? */
    if (inode->u.i_ops != NULL && inode->u.i_ops->ioctl != NULL) {
        /* Yes on both accounts.  Let the driver perform the ioctl command */
        ret = inode->u.i_ops->ioctl(filep, req, arg);
    }

    /* Check for File system IOCTL commands that can be implemented via
    * fcntl()
    */
    if (ret != -ENOTTY) {
        return ret;
    }

    switch (req) {
    case FIONBIO:
        {
            int *nonblock = (int *)(uintptr_t)arg;
            if (nonblock && *nonblock) {
                filep->f_oflags |= O_NONBLOCK;
            } else {
                filep->f_oflags &= ~O_NONBLOCK;
            }

            ret = 0; // OK
        }
        break;

    case FIOCLEX:
        filep->f_oflags |= O_CLOEXEC;
        ret = 0; // OK
        break;

    case FIONCLEX:
        filep->f_oflags &= ~O_CLOEXEC;
        ret = 0; // OK
        break;

    case FIOC_FILEPATH:
        if (!INODE_IS_MOUNTPT(inode)) {
            ret = inode_getpath(inode, (char *)(uintptr_t)arg);
        }
        break;

    case BIOC_BLKSSZGET:
        if (inode->u.i_ops != NULL && inode->u.i_ops->ioctl != NULL) {
            struct geometry geo;
            ret = inode->u.i_ops->ioctl(filep, BIOC_GEOMETRY,
                                        (unsigned long)(uintptr_t)&geo);
            if (ret >= 0) {
                *(blksize_t *)(uintptr_t)arg = geo.geo_sectorsize;
            }
        }
    }

    return ret;
}

static int nx_vioctl(int fd, int req, va_list ap)
{
    struct file *filep;
    int ret;

    ret = fs_getfilep(fd, &filep);
    if (ret < 0) {
        return ret;
    }

    return file_vioctl(filep, req, ap);
}

int file_ioctl(struct file *filep, int req, ...)
{
    va_list ap;
    int ret;

    va_start(ap, req);
    ret = file_vioctl(filep, req, ap);
    va_end(ap);

    return ret;
}

int nx_ioctl(int fd, int req, ...)
{
    va_list ap;
    int ret;

    va_start(ap, req);
    ret = nx_vioctl(fd, req, ap);
    va_end(ap);

    return ret;
}

int ioctl(int fd, int req, ...)
{
    va_list ap;
    int ret;

    va_start(ap, req);
    ret = nx_vioctl(fd, req, ap);
    va_end(ap);

    if (ret < 0) {
        set_errno(-ret);
        ret = -1; // ERROR
    }

    return ret;
}
