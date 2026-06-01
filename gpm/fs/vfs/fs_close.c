#include <unistd.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

/****************************************************************************
 * Name: file_close
 *
 * Description:
 *   Close a file that was previously opened with file_open().
 *
 * Input Parameters:
 *   filep - A pointer to a user provided memory location containing the
 *           open file data returned by file_open().
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned on
 *   any failure to indicate the nature of the failure.
 *
 ****************************************************************************/
int file_close(struct file *filep)
{
    struct inode *inode;
    int ret = 0;

    inode = filep->f_inode;

    /* Check if the struct file is open (i.e., assigned an inode) */
    if (inode) {

        /* Close the file, driver, or mountpoint. */
        if (inode->u.i_ops && inode->u.i_ops->close) {
            ret = inode->u.i_ops->close(filep);
        }

        /* And release the inode */
        inode_release(inode);

        /* Reset the user file struct instance so that it cannot be reused. */
        filep->f_oflags = 0;
        filep->f_pos    = 0;
        filep->f_inode  = NULL;
        filep->f_priv   = NULL;
    }

    return ret;
}
