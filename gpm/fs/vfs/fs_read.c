#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include "inode/inode.h"

/****************************************************************************
 * Name: file_read
 *
 * Description:
 *   file_read() is an internal OS interface.  It is functionally similar to
 *   the standard read() interface except:
 *
 *    - It does not modify the errno variable,
 *    - It is not a cancellation point,
 *    - It accepts a file structure instance instead of file descriptor.
 *
 * Input Parameters:
 *   filep  - File structure instance
 *   buf    - User-provided to save the data
 *   nbytes - The maximum size of the user-provided buffer
 *
 * Returned Value:
 *   The positive non-zero number of bytes read on success, 0 on if an
 *   end-of-file condition, or a negated errno value on any failure.
 *
 ****************************************************************************/
ssize_t file_read(struct file *filep, void *buf, size_t nbytes)
{
    struct inode *inode;
    int ret = -EBADF;

    inode = filep->f_inode;

    /* Was this file opened for read access? */
    if ((filep->f_oflags & O_RDOK) == 0) {
        ret = -EACCES;

    /* Is a driver or mountpoint registered? If so, does it support the read
    * method?
    */
    } else if (inode != NULL && inode->u.i_ops && inode->u.i_ops->read) {

        /* Yes.. then let it perform the read.  NOTE that for the case of the
        * mountpoint, we depend on the read methods being identical in
        * signature and position in the operations vtable.
        */
        ret = (int)inode->u.i_ops->read(filep,
                                        (char *)buf,
                                        (size_t)nbytes);
    }

    return ret;
}

ssize_t nx_read(int fd, void *buf, size_t nbytes)
{
    struct file *filep;
    ssize_t ret;

    ret = (ssize_t)fs_getfilep(fd, &filep);
    if (ret < 0) {
        return ret;
    }

    return file_read(filep, buf, nbytes);
}

ssize_t read(int fd, void *buf, size_t nbytes)
{
    ssize_t ret;

    ret = nx_read(fd, buf, nbytes);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1; // ERROR
    }

    return ret;
}
