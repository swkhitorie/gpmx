#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "inode/inode.h"

/****************************************************************************
 * Name: file_dup2
 *
 * Description:
 *   Assign an inode to a specific files structure.  This is the heart of
 *   dup2.
 *
 *   Equivalent to the non-standard dup2() function except that it
 *   accepts struct file instances instead of file descriptors and it does
 *   not set the errno variable.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is return on
 *   any failure.
 *
 ****************************************************************************/
int file_dup2(struct file *filep1, struct file *filep2)
{
    struct inode *inode;
    struct file temp;
    int ret;

    if (filep1 == NULL || filep1->f_inode == NULL || filep2 == NULL) {
        return -EBADF;
    }

    if (filep1 == filep2) {
        return 0;
    }

    /* Increment the reference count on the contained inode */
    inode = filep1->f_inode;
    ret   = inode_addref(inode);

    if (ret < 0) {
        return ret;
    }

    /* Then clone the file structure */
    temp.f_oflags = filep1->f_oflags;
    temp.f_pos    = filep1->f_pos;
    temp.f_inode  = inode;
    temp.f_priv   = NULL;

    /* Call the open method on the file, driver, mountpoint so that it
    * can maintain the correct open counts.
    */
    if (inode->u.i_ops) {
        if (INODE_IS_MOUNTPT(inode)) {

            /* Dup the open file on the in the new file structure */
            if (inode->u.i_mops->dup) {
                ret = inode->u.i_mops->dup(filep1, &temp);
            }
        } else {

            /* (Re-)open the pseudo file or device driver */
            temp.f_priv = filep1->f_priv;
            if (inode->u.i_ops->open) {
                ret = inode->u.i_ops->open(&temp);
            }
        }

        if (ret < 0) {
            inode_release(inode);
            return ret;
        }
    }

    /* If there is already an inode contained in the new file structure,
    * close the file and release the inode.
    */
    ret = file_close(filep2);
    memcpy(filep2, &temp, sizeof(temp));
    return 0;
}
