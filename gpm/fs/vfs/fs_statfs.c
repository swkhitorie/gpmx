#include <sys/statfs.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "inode/inode.h"

static int statpseudofs(struct inode *inode, struct statfs *buf)
{
    memset(buf, 0, sizeof(struct statfs));
    buf->f_type    = PROC_SUPER_MAGIC;
    buf->f_namelen = NAME_MAX;
    return 0; // OK
}

int statfs(const char *path, struct statfs *buf)
{
    struct inode_search_s desc;
    struct inode *inode;
    int ret = 0; // OK

    if (path == NULL  || buf == NULL) {
        ret = -EFAULT;
        goto errout;
    }

    if (*path == '\0') {
        ret = -ENOENT;
        goto errout;
    }

    SETUP_SEARCH(&desc, path, false);

    ret = inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;

    if (INODE_IS_MOUNTPT(inode)) {
        if (inode->u.i_mops && inode->u.i_mops->statfs) {
            ret = inode->u.i_mops->statfs(inode, buf);
        }
    } else {
        ret = statpseudofs(inode, buf);
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

    inode_release(inode);
    RELEASE_SEARCH(&desc);
    return 0; // OK

errout_with_inode:
    inode_release(inode);

errout_with_search:
    RELEASE_SEARCH(&desc);

errout:
    set_errno(-ret);
    return -1; // ERROR
}
