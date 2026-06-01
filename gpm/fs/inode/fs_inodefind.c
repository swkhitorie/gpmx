#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

int inode_find(struct inode_search_s *desc)
{
    int ret;

    ret = inode_semtake();
    if (ret < 0) {
        return ret;
    }

    ret = inode_search(desc);
    if (ret >= 0) {
        struct inode *node = desc->node;
        node->i_crefs++;
    }

    inode_semgive();
    return ret;
}
