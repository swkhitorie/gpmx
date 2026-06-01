#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "inode/inode.h"
#include "gpm/fs/ioctl.h"
#include "gpm/mtd/mtd.h"

#define RESET_BUF(b) memset((b), 0, sizeof(struct stat));

static int stat_recursive(const char *path,
                          struct stat *buf, int resolve)
{
    struct inode_search_s desc;
    struct inode *inode;
    int ret;

    SETUP_SEARCH(&desc, path, true);

    ret = inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;

    if (INODE_IS_MOUNTPT(inode)) {

        if (inode->u.i_mops && inode->u.i_mops->stat) {
            ret = inode->u.i_mops->stat(inode, desc.relpath, buf);
        } else {
            ret = -ENOSYS;
        }
    } else {
        ret = inode_stat(inode, buf, resolve);
    }

    inode_release(inode);
errout_with_search:
    RELEASE_SEARCH(&desc);
    return ret;
}

int nx_stat(const char *path, struct stat *buf, int resolve)
{
    if (path == NULL  || buf == NULL) {
        return -EFAULT;
    }

    if (*path == '\0') {
        return -ENOENT;
    }

    return stat_recursive(path, buf, resolve);
}

int stat(const char *path, struct stat *buf)
{
    int ret;

    ret = nx_stat(path, buf, 1);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1; // ERROR
    }

    return ret;
}

int lstat(const char *path, struct stat *buf)
{
    int ret;

    ret = nx_stat(path, buf, 0);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1; // ERROR
    }

    return ret;
}

int inode_stat(struct inode *inode, struct stat *buf, int resolve)
{
    RESET_BUF(buf);

    if (INODE_IS_MTD(inode)) {
        struct mtd_geometry_s mtdgeo;

        buf->st_mode  = S_IFMTD;
        buf->st_mode |= S_IROTH | S_IRGRP | S_IRUSR;
        buf->st_mode |= S_IWOTH | S_IWGRP | S_IWUSR;

        if (inode->u.i_mtd != NULL &&
            MTD_IOCTL(inode->u.i_mtd, MTDIOC_GEOMETRY,
                    (unsigned long)((uintptr_t)&mtdgeo)) >= 0)
        {
            buf->st_size = mtdgeo.neraseblocks * mtdgeo.erasesize;
        }
    } else if (inode->u.i_ops != NULL) {
        if (inode->u.i_ops->read) {
            buf->st_mode = S_IROTH | S_IRGRP | S_IRUSR;
        }

        if (inode->u.i_ops->write) {
            buf->st_mode |= S_IWOTH | S_IWGRP | S_IWUSR;
        }

        if (INODE_IS_MOUNTPT(inode)) {
            buf->st_mode |= S_IFDIR;
        } else if (INODE_IS_BLOCK(inode)) {
            buf->st_mode |= S_IFBLK;

            if ((inode->u.i_bops != NULL) &&
                (inode->u.i_bops->geometry != NULL))
            {
                struct geometry geo;
                if (inode->u.i_bops->geometry(inode, &geo) >= 0 &&
                    geo.geo_available)
                {
                  buf->st_size = geo.geo_nsectors * geo.geo_sectorsize;
                }
            }
        } else  {
            /* if (INODE_IS_DRIVER(inode)) */
            buf->st_mode |= S_IFCHR;
        }
    } else {
        buf->st_mode |= S_IFDIR | S_IROTH | S_IRGRP | S_IRUSR;
    }

    return 0; // OK
}
