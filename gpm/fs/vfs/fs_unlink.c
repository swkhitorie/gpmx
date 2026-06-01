#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int nx_unlink(const char *pathname)
{
    struct inode_search_s desc;
    struct inode *inode;
    int ret;

    SETUP_SEARCH(&desc, pathname, true);

    ret = inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;

    if (INODE_IS_MOUNTPT(inode) && inode->u.i_mops) {

        if (inode->u.i_mops->unlink) {
            ret = inode->u.i_mops->unlink(inode, desc.relpath);
            if (ret < 0) {
                goto errout_with_inode;
            }
        } else {
            ret = -ENOSYS;
            goto errout_with_inode;
        }
    } else {

        if (inode->i_child != NULL) {
            ret = -ENOTEMPTY;
            goto errout_with_inode;
        }

        if (INODE_IS_DRIVER(inode) && inode->u.i_ops->unlink) {
            ret = inode->u.i_ops->unlink(inode);
            if (ret < 0) {
                goto errout_with_inode;
            }
        } else if (INODE_IS_BLOCK(inode) && inode->u.i_bops->unlink) {

            ret = inode->u.i_bops->unlink(inode);
            if (ret < 0) {
                goto errout_with_inode;
            }
        } else if (INODE_IS_PSEUDODIR(inode)) {
            // do nothing
        } else {
            ret = -ENXIO;
            goto errout_with_inode;
        }


        ret = inode_semtake();
        if (ret < 0) {
            goto errout_with_inode;
        }

        ret = inode_remove(pathname);
        inode_semgive();

        if (ret < 0 && ret != -EBUSY) {
            goto errout_with_inode;
        }
    }

    inode_release(inode);
    RELEASE_SEARCH(&desc);
    return 0; // OK

errout_with_inode:
    inode_release(inode);

errout_with_search:
    RELEASE_SEARCH(&desc);
    return ret;
}

int unlink(const char *pathname)
{
    int ret;

    ret = nx_unlink(pathname);
    if (ret < 0) {
        set_errno(-ret);
        return -1; // ERROR
    }

    return 0; // OK
}
