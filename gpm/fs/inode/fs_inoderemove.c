#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

struct inode *inode_unlink(const char *path)
{
    struct inode_search_s desc;
    struct inode *node = NULL;
    int ret;

    if (path == NULL) {
        return NULL;
    }

    SETUP_SEARCH(&desc, path, true);

    /* Find the node to unlink */
    ret = inode_search(&desc);
    if (ret >= 0) {
        node = desc.node;

        /* If peer is non-null, then remove the node from the right of
        * of that peer node.
        */
        if (desc.peer != NULL) {
            desc.peer->i_peer = node->i_peer;

        /* Then remove the node from head of the list of children. */
        } else {
            desc.parent->i_child = node->i_peer;
        }

        node->i_peer   = NULL;
        node->i_parent = NULL;
    }

    RELEASE_SEARCH(&desc);
    return node;
}

int inode_remove(const char *path)
{
    struct inode *node;

    /* Find the inode and unlink it from the in-memory inode tree */
    node = inode_unlink(path);
    if (node) {

        /* Found it! But we cannot delete the inode if there are references to it */
        if (node->i_crefs) {
            /* In that case, we will mark it deleted, when the filesystem
             * releases the inode, we will then, finally delete the subtree
             */
            node->i_flags |= FSNODEFLAG_DELETED;
            return -EBUSY;
        } else {
            /* And delete it now -- recursively to delete all of its children.
            * Since it has been unlinked, then the peer pointer should be
            * NULL.
            */
            inode_free(node);
            return 0;
        }
    }

    return -ENOENT;
}
