
#include "gpm/fs/fs.h"
#include "inode/inode.h"

int unregister_driver(const char *path)
{
    int ret;

    ret = inode_semtake();
    if (ret >= 0) {
        ret = inode_remove(path);
        inode_semgive();
    }

    return ret;
}
