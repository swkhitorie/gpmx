#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"


void inode_free(struct inode *node)
{
    if (node != NULL) {

        inode_free(node->i_peer);
        inode_free(node->i_child);

        kmm_free(node);
    }
}
