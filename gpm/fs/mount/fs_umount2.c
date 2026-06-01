#include <sys/mount.h>
#include <stdbool.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int nx_umount2(const char *target, unsigned int flags)
{
    struct inode *mountpt_inode;
    struct inode *blkdrvr_inode = NULL;
    struct inode_search_s desc;
    int ret;

    /* Verify required pointer arguments */

    if (!target) {
        ret = -EFAULT;
        goto errout;
    }

    /* Find the mountpt */
    SETUP_SEARCH(&desc, target, false);

    ret = inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    /* Get the search results */
    mountpt_inode = desc.node;

    /* Verify that the inode is a mountpoint */
    if (!INODE_IS_MOUNTPT(mountpt_inode)) {
        ret = -EINVAL;
        goto errout_with_mountpt;
    }

    /* Unbind the block driver from the file system (destroying any fs
    * private data.
    */

    if (!mountpt_inode->u.i_mops->unbind) {
        /* The filesystem does not support the unbind operation ??? */
        ret = -EINVAL;
        goto errout_with_mountpt;
    }

    /* The unbind method returns the number of references to the
    * filesystem (i.e., open files), zero if the unbind was
    * performed, or a negated error code on a failure.
    */

    /* Hold the semaphore through the unbind logic */

    ret = inode_semtake();
    if (ret < 0) {
        goto errout_with_mountpt;
    }

    ret = mountpt_inode->u.i_mops->unbind(mountpt_inode->i_private,
                                        &blkdrvr_inode, flags);
    if (ret < 0) {
        /* The inode is unhappy with the blkdrvr for some reason */
        goto errout_with_semaphore;
    } else if (ret > 0) {
        ret = -EBUSY;
        goto errout_with_semaphore;
    }

    /* Successfully unbound.  Convert the mountpoint inode to regular
    * pseudo-file inode.
    */

    mountpt_inode->i_flags  &= ~FSNODEFLAG_TYPE_MASK;
    mountpt_inode->i_private = NULL;
    mountpt_inode->u.i_mops  = NULL;

    /* If the node has children, then do not delete it. */
    if (mountpt_inode->i_child != NULL) {

      /* Just decrement the reference count (without deleting it) */
        mountpt_inode->i_crefs--;
        inode_semgive();
    } else {
        /* Remove the mountpoint inode from the inode tree.  The inode will
        * not be deleted yet because there is still at least reference on
        * it (from the mount)
        */

        ret = inode_remove(target);
        inode_semgive();

        /* The return value of -EBUSY is normal (in fact, it should
        * not be OK)
        */

        if (ret != 0 /* OK */ && ret != -EBUSY) {
            goto errout_with_mountpt;
        }

        /* Release the mountpoint inode and any block driver inode
        * returned by the file system unbind above.  This should cause
        * the inode to be deleted (unless there are other references)
        */
        inode_release(mountpt_inode);
    }

    /* Did the unbind method return a contained block driver */
    if (blkdrvr_inode) {
        inode_release(blkdrvr_inode);
    }

    RELEASE_SEARCH(&desc);
    return 0; /* OK */

    /* A lot of goto's!  But they make the error handling much simpler */

errout_with_semaphore:
    inode_semgive();

errout_with_mountpt:
    inode_release(mountpt_inode);
    if (blkdrvr_inode) {
        inode_release(blkdrvr_inode);
    }

errout_with_search:
    RELEASE_SEARCH(&desc);

errout:
    return ret;
}

int umount2(const char *target, unsigned int flags)
{
    int ret;

    ret = nx_umount2(target, flags);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1; /* ERROR */
    }

    return ret;
}
