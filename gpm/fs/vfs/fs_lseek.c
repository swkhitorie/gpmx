#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "inode/inode.h"

/****************************************************************************
 * Name: file_seek
 *
 * Description:
 *   This is the internal implementation of lseek.  See the comments in
 *   lseek() for further information.
 *
 * Input Parameters:
 *   file     File structure instance
 *   offset   Defines the offset to position to
 *   whence   Defines how to use offset
 *
 * Returned Value:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure (see lseek comments).
 *
 ****************************************************************************/
off_t file_seek(struct file *filep, off_t offset, int whence)
{
    struct inode *inode;
    off_t ret;

    inode =  filep->f_inode;

    /* Invoke the file seek method if available */
    if (inode && inode->u.i_ops && inode->u.i_ops->seek) {
        ret = inode->u.i_ops->seek(filep, offset, whence);
        if (ret < 0) {
            return ret;
        }
    } else {

        /* No... Just set the common file position value */
        switch (whence) {
        case SEEK_CUR:
            offset += filep->f_pos;
        case SEEK_SET:
            if (offset >= 0) {
                filep->f_pos = offset; /* Might be beyond the end-of-file */
                break;
            } else {
                return -EINVAL;
            }
            break;
        case SEEK_END:
            return -ENOSYS;
        default:
            return -EINVAL;
        }
    }

    return filep->f_pos;
}

off_t nx_seek(int fd, off_t offset, int whence)
{
    struct file *filep;
    off_t ret;

    ret = fs_getfilep(fd, &filep);
    if (ret < 0) {
        return ret;
    }

    return file_seek(filep, offset, whence);
}


off_t lseek(int fd, off_t offset, int whence)
{
    off_t newpos;

    newpos = nx_seek(fd, offset, whence);
    if (newpos < 0) {
        set_errno(-newpos);
        return -1; // ERROR
    }

    return newpos;
}
