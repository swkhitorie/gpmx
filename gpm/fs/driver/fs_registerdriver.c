#include <sys/types.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int register_driver(const char *path,
                    const struct file_operations *fops,
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
        INODE_SET_DRIVER(node);

        node->u.i_ops   = fops;
        node->i_private = priv;
        ret             = 0;
    }

    inode_semgive();
    return ret;
}
