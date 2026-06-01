#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int close_blockdriver(struct inode *inode)
{
    int ret = 0;

    if (!INODE_IS_BLOCK(inode)) {
        ret = -ENOTBLK;
        goto errout;
    }

    if (inode->u.i_bops->close) {
        ret = inode->u.i_bops->close(inode);
    }

    inode_release(inode);

errout:
    return ret;
}
