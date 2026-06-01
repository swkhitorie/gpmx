
#include <sys/types.h>
#include <sys/mount.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"

#include "inode/inode.h"
#include "driver/driver.h"

int find_blockdriver(const char *pathname, int mountflags,
                    struct inode **ppinode)
{
    struct inode_search_s desc;
    struct inode *inode;
    int ret = 0;

    SETUP_SEARCH(&desc, pathname, false);

    ret = inode_find(&desc);
    if (ret < 0) {
        ret = -ENOENT;
        goto errout_with_search;
    }

    inode = desc.node;

    if (!INODE_IS_BLOCK(inode)) {
        ret = -ENOTBLK;
        goto errout_with_inode;
    }

    if (!inode->u.i_bops || !inode->u.i_bops->read ||
        (!inode->u.i_bops->write && (mountflags & MS_RDONLY) == 0))
    {
        ret = -EACCES;
        goto errout_with_inode;
    }

    *ppinode = inode;
    RELEASE_SEARCH(&desc);
    return 0;

errout_with_inode:
    inode_release(inode);
errout_with_search:
    RELEASE_SEARCH(&desc);
    return ret;
}
