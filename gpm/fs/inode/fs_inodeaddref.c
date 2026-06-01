#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int inode_addref(struct inode *inode)
{
    int ret = 0;

    if (inode) {
        ret = inode_semtake();
        if (ret >= 0) {
            inode->i_crefs++;
            inode_semgive();
        }
    }

    return ret;
}
