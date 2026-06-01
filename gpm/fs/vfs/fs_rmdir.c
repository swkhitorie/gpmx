#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"

#include "inode/inode.h"

int rmdir(const char *pathname)
{
    struct inode_search_s desc;
    struct inode *inode;
    int errcode;
    int ret;


    SETUP_SEARCH(&desc, pathname, true);

    ret = inode_find(&desc);
    if (ret < 0) {
        /* There is no inode that includes in this path */
        errcode = -ret;
        goto errout_with_search;
    }

    /* Get the search results */
    inode = desc.node;

    /* Check if the inode is a valid mountpoint. */

    if (INODE_IS_MOUNTPT(inode) && inode->u.i_mops) {
        /* Perform the rmdir operation using the relative path
        * from the mountpoint.
        */

        if (inode->u.i_mops->rmdir) {
            ret = inode->u.i_mops->rmdir(inode, desc.relpath);
            if (ret < 0) {
                errcode = -ret;
                goto errout_with_inode;
            }
        } else {
            errcode = ENOSYS;
            goto errout_with_inode;
        }

    /* If this is a "dangling" pseudo-directory node (i.e., it has no
    * operations) then rmdir should remove the node.
    */
    } else if (!inode->u.i_ops) {
        /* If the directory inode has children, however, then it cannot be
        * removed.
        */
        if (inode->i_child) {
            errcode = ENOTEMPTY;
            goto errout_with_inode;
        }

        /* Remove the inode.  NOTE: Because we hold a reference count on the
        * inode, it will not be deleted now.  But probably when
        * inode_release() is called below.  inode_remove should return
        * -EBUSY to indicate that the inode was not deleted now.
        */

        ret = inode_semtake();
        if (ret < 0) {
            errcode = -ret;
            goto errout_with_inode;
        }

        ret = inode_remove(pathname);
        inode_semgive();

        if (ret < 0 && ret != -EBUSY) {
            errcode = -ret;
            goto errout_with_inode;
        }
    } else {
        errcode = ENOTDIR;
        goto errout_with_inode;
    }

    inode_release(inode);
    RELEASE_SEARCH(&desc);
    return 0; // OK
errout_with_inode:
    inode_release(inode);
errout_with_search:
    RELEASE_SEARCH(&desc);
    set_errno(errcode);
    return -1; // ERROR
}
