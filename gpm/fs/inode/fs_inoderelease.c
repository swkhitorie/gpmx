#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

void inode_release(struct inode *node)
{
    int ret;

    if (node) {

        do {
            ret = inode_semtake();
        } while (ret < 0);

        if (node->i_crefs) {
            node->i_crefs--;
        }

        if (node->i_crefs <= 0 && (node->i_flags & FSNODEFLAG_DELETED) != 0) {
            inode_semgive();
            inode_free(node);
        } else {
            inode_semgive();
        }
    }
}
