#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

static int inode_namelen(const char *name)
{
    const char *tmp = name;
    while (*tmp && *tmp != '/') {
        tmp++;
    }

    return tmp - name;
}

static void inode_namecpy(char *dest, const char *src)
{
    while (*src && *src != '/') {
        *dest++ = *src++;
    }

    *dest = '\0';
}

static struct inode *inode_alloc(const char *name, mode_t mode)
{
    struct inode *node;
    int namelen;

    namelen = inode_namelen(name);
    node    = (struct inode *)kmm_zalloc(FSNODE_SIZE(namelen));
    if (node) {
        inode_namecpy(node->i_name, name);
    }

    return node;
}

static void inode_insert(struct inode *node,
                         struct inode *peer,
                         struct inode *parent)
{
    /* If peer is non-null, then new node simply goes to the right
    * of that peer node.
    */
    if (peer) {
        node->i_peer   = peer->i_peer;
        node->i_parent = parent;
        peer->i_peer   = node;

    /* Then it must go at the head of parent's list of children. */
    } else {
        node->i_peer    = parent->i_child;
        node->i_parent  = parent;
        parent->i_child = node;
    }
}

void inode_root_reserve(void)
{
    g_root_inode = inode_alloc("", 0777);
}

int inode_reserve(const char *path,
                  mode_t mode, struct inode **inode)
{
    struct inode_search_s desc;
    struct inode *left;
    struct inode *parent;
    const char *name;
    int ret;

    *inode = NULL;

    if (path[0] == '\0') {
        return -EINVAL;
    }

    /* Find the location to insert the new subtree */
    SETUP_SEARCH(&desc, path, false);

    ret = inode_search(&desc);
    if (ret >= 0) {

        /* It is an error if the node already exists in the tree (or if it
        * lies within a mountpoint, we don't distinguish here).
        */
        ret = -EEXIST;
        goto errout_with_search;
    }

    /* Now we now where to insert the subtree */
    name   = desc.path;
    left   = desc.peer;
    parent = desc.parent;

    for (;;) {
        struct inode *node;

        /* Create a new node -- we need to know if this is the
        * the leaf node or some intermediary.  We can find this
        * by looking at the next name.
        */
        const char *nextname = inode_nextname(name);
        if (*nextname != '\0') {
          /* Insert an operationless node */

            node = inode_alloc(name, 0777);
            if (node != NULL) {
                inode_insert(node, left, parent);

                /* Set up for the next time through the loop */
                name   = nextname;
                left   = NULL;
                parent = node;
                continue;
            }
        } else {
            node = inode_alloc(name, mode);
            if (node != NULL) {
                inode_insert(node, left, parent);
                *inode = node;
                ret = 0;
                break;
            }
        }

        /* We get here on failures to allocate node memory */
        ret = -ENOMEM;
        break;
    }

errout_with_search:
    RELEASE_SEARCH(&desc);
    return ret;
}
