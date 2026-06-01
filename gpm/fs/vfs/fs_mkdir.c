#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int mkdir(const char *pathname, mode_t mode)
{
    struct inode_search_s desc;
    struct inode *inode;
    int errcode;
    int ret;

    mode &= ~getumask();

    SETUP_SEARCH(&desc, pathname, false);

    ret = inode_find(&desc);
    if (ret >= 0) {

        /* An inode was found that includes this path and possibly refers to a
        * mountpoint.
        */
        inode = desc.node;

        if (desc.relpath[0] == '\0') {
            errcode = EEXIST;
            goto errout_with_inode;
        }

        /* Check if the inode is a valid mountpoint. */
        if (!INODE_IS_MOUNTPT(inode) || !inode->u.i_mops) {
            errcode = ENXIO;
            goto errout_with_inode;
        }

        /* Perform the mkdir operation using the relative path
        * at the mountpoint.
        */
        if (inode->u.i_mops->mkdir) {
            ret = inode->u.i_mops->mkdir(inode, desc.relpath, mode);
            if (ret < 0) {
                errcode = -ret;
                goto errout_with_inode;
            }
        } else {
            errcode = ENOSYS;
            goto errout_with_inode;
        }

        inode_release(inode);
    } else {

        /* Create an inode in the pseudo-filesystem at this path.
        * NOTE that the new inode will be created with a reference
        * count of zero.
        */
        ret = inode_semtake();
        if (ret < 0) {
            errcode = -ret;
            goto errout_with_search;
        }

        ret = inode_reserve(pathname, mode, &inode);
        inode_semgive();

        if (ret < 0) {
            errcode = -ret;
            goto errout_with_search;
        }
    }

    RELEASE_SEARCH(&desc);
    return 0; // OK

errout_with_inode:
    inode_release(inode);

errout_with_search:
    RELEASE_SEARCH(&desc);
    set_errno(errcode);
    return -1; // ERROR
}
