#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

/****************************************************************************
 * Name: file_dup
 *
 * Description:
 *   Equivalent to the standard dup() function except that it
 *   accepts a struct file instance instead of a file descriptor.
 *
 * Returned Value:
 *   The new file descriptor is returned on success; a negated errno value
 *   is returned on any failure.
 *
 ****************************************************************************/
int file_dup(struct file *filep, int minfd)
{
    struct file filep2;
    int fd2;
    int ret;

    memset(&filep2, 0, sizeof(filep2));
    ret = file_dup2(filep, &filep2);

    if (ret < 0) {
        return ret;
    }

    /* Then allocate a new file descriptor for the inode */
    fd2 = files_allocate(filep2.f_inode, filep2.f_oflags,
                        filep2.f_pos, filep2.f_priv, minfd);
    if (fd2 < 0) {
        file_close(&filep2);
        return fd2;
    }

    return fd2;
}

int nx_dup(int fd)
{
    struct file *filep;
    int ret;

    ret = fs_getfilep(fd, &filep);
    if (ret < 0) {
        return ret;
    }

    return file_dup(filep, 0);
}

/****************************************************************************
 * Name: dup
 *
 * Description:
 *   Clone a file or socket descriptor to an arbitrary descriptor number
 *
 ****************************************************************************/
int dup(int fd)
{
    int ret;

    ret = nx_dup(fd);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1;
    }

    return ret;
}
