#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include "inode/inode.h"

ssize_t file_write(struct file *filep, const void *buf,
                    size_t nbytes)
{
    struct inode *inode;

    if ((filep->f_oflags & O_WROK) == 0) {
        return -EACCES;
    }

    inode = filep->f_inode;
    if (!inode || !inode->u.i_ops || !inode->u.i_ops->write) {
        return -EBADF;
    }

    return inode->u.i_ops->write(filep, buf, nbytes);
}

ssize_t nx_write(int fd, const void *buf, size_t nbytes)
{
    struct file *filep;
    ssize_t ret;

    if (buf == NULL) {
        return -EINVAL;
    }

    ret = (ssize_t)fs_getfilep(fd, &filep);
    if (ret >= 0) {
        ret = file_write(filep, buf, nbytes);
    }

    return ret;
}

ssize_t write(int fd, const void *buf, size_t nbytes)
{
    ssize_t ret;

    ret = nx_write(fd, buf, nbytes);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1; // ERROR
    }

    return ret;
}
