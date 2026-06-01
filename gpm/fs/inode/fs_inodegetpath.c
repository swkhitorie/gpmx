#include <errno.h>
#include <string.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

/** Given the full path from inode. */
int inode_getpath(struct inode *node, char *path)
{
    if (path == NULL) {
        return -EINVAL; 
    } else if (node == NULL) {
        path[0] = '\0';
        return 0;
    } else {
        int ret = inode_getpath(node->i_parent, path);
        if (ret < 0) {
            return ret;
        }
    }

    strcat(path, node->i_name);
    if (node->i_child || INODE_IS_MOUNTPT(node)) {
        strcat(path, "/");
    }

    return 0;
}
