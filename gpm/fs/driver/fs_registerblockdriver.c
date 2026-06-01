#include <sys/types.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int register_blockdriver(const char *path,
                         const struct block_operations *bops,
                         mode_t mode, void *priv)
{
    struct inode *node;
    int ret;

    ret = inode_semtake();
    if (ret < 0) {
        return ret;
    }

    ret = inode_reserve(path, mode, &node);
    if (ret >= 0) {
        INODE_SET_BLOCK(node);

        node->u.i_bops  = bops;
        node->i_private = priv;
        ret             = 0;
    }

    inode_semgive();
    return ret;
}

