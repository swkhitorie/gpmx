#include <errno.h>
#include "gpm/fs/fs.h"

#include "inode/inode.h"
#include "driver/driver.h"

int open_blockdriver(const char *pathname, int mountflags,
                    struct inode **ppinode)
{
    struct inode *inode;
    int ret;

    ret = find_blockdriver(pathname, mountflags, &inode);
    if (ret < 0) {
        /* Not block device, mtd device? let's try it. */
        return mtd_proxy(pathname, mountflags, ppinode);
    }

    if (inode->u.i_bops->open){
        ret = inode->u.i_bops->open(inode);
        if (ret < 0) {
            inode_release(inode);
            return ret;
        }
    }

    *ppinode = inode;
    return 0;
}
