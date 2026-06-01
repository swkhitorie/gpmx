#include <sys/mount.h>

#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"
#include "driver/driver.h"

/* In the canonical case, a file system is bound to a block driver.  However,
 * some less typical cases a block driver is not required.  Examples are
 * pseudo file systems (like BINFS or PROCFS) and MTD file systems (like
 * NXFFS).
 *
 * These file systems all require block drivers:
 */

#if defined(CONFIG_FS_FAT) || defined(CONFIG_FS_ROMFS) || \
    defined(CONFIG_FS_LITTLEFS)
#  define BDFS_SUPPORT 1
#endif

/* These file systems require MTD drivers */
#if defined(CONFIG_FS_LITTLEFS)
#  define MDFS_SUPPORT 1
#endif

/* These file systems do not require block or MTD drivers */
#if defined(CONFIG_FS_NXFFS) || defined(CONFIG_FS_BINFS) || \
    defined(CONFIG_FS_PROCFS) || defined(CONFIG_NFS) || \
    defined(CONFIG_FS_TMPFS) || defined(CONFIG_FS_USERFS) || \
    defined(CONFIG_FS_CROMFS) || defined(CONFIG_FS_UNIONFS) || \
    defined(CONFIG_FS_HOSTFS)
#  define NODFS_SUPPORT
#endif

struct fsmap_t
{
  const char                      *fs_filesystemtype;
  const struct mountpt_operations *fs_mops;
};


#ifdef BDFS_SUPPORT
/* File systems that require block drivers */

#ifdef CONFIG_FS_FAT
extern const struct mountpt_operations fat_operations;
#endif
#ifdef CONFIG_FS_ROMFS
extern const struct mountpt_operations romfs_operations;
#endif
#ifdef CONFIG_FS_LITTLEFS
extern const struct mountpt_operations littlefs_operations;
#endif

static const struct fsmap_t g_bdfsmap[] =
{
#ifdef CONFIG_FS_FAT
    { "vfat", &fat_operations },
#endif
#ifdef CONFIG_FS_ROMFS
    { "romfs", &romfs_operations },
#endif
#ifdef CONFIG_FS_LITTLEFS
    { "littlefs", &littlefs_operations },
#endif
    { NULL,   NULL },
};
#endif /* BDFS_SUPPORT */

#ifdef MDFS_SUPPORT
/* File systems that require MTD drivers */

#ifdef CONFIG_FS_ROMFS
extern const struct mountpt_operations romfs_operations;
#endif
#ifdef CONFIG_FS_LITTLEFS
extern const struct mountpt_operations littlefs_operations;
#endif

static const struct fsmap_t g_mdfsmap[] =
{
#ifdef CONFIG_FS_ROMFS
    { "romfs", &romfs_operations },
#endif
#ifdef CONFIG_FS_LITTLEFS
    { "littlefs", &littlefs_operations },
#endif
    { NULL,   NULL },
};
#endif /* MDFS_SUPPORT */

#ifdef NODFS_SUPPORT
/* File systems that require neither block nor MTD drivers */

#ifdef CONFIG_FS_TMPFS
extern const struct mountpt_operations tmpfs_operations;
#endif
#ifdef CONFIG_FS_BINFS
extern const struct mountpt_operations binfs_operations;
#endif
#ifdef CONFIG_FS_PROCFS
extern const struct mountpt_operations procfs_operations;
#endif

static const struct fsmap_t g_nonbdfsmap[] =
{
#ifdef CONFIG_FS_TMPFS
    { "tmpfs", &tmpfs_operations },
#endif
#ifdef CONFIG_FS_BINFS
    { "binfs", &binfs_operations },
#endif
#ifdef CONFIG_FS_PROCFS
    { "procfs", &procfs_operations },
#endif
    { NULL, NULL },
};
#endif /* NODFS_SUPPORT */

#if defined(BDFS_SUPPORT) || defined(MDFS_SUPPORT) || defined(NODFS_SUPPORT)
static const struct mountpt_operations *
mount_findfs(const struct fsmap_t *fstab, const char *filesystemtype)
{
    const struct fsmap_t *fsmap;

    for (fsmap = fstab; fsmap->fs_filesystemtype; fsmap++) {
        if (strcmp(filesystemtype, fsmap->fs_filesystemtype) == 0) {
            return fsmap->fs_mops;
        }
    }
    return NULL;
}
#endif

int nx_mount(const char *source, const char *target,
             const char *filesystemtype, unsigned long mountflags,
             const void *data)
{
#if defined(BDFS_SUPPORT) || defined(MDFS_SUPPORT) || defined(NODFS_SUPPORT)
    struct inode *drvr_inode = NULL;
    struct inode *mountpt_inode;
    const struct mountpt_operations *mops = NULL;
    struct inode_search_s desc;
    void *fshandle;
    int ret;

    /* Find the specified filesystem. Try the block driver filesystems first */
    if (source != NULL &&
        find_blockdriver(source, mountflags, &drvr_inode) >= 0)
    {
        /* Find the block based file system */

#ifdef BDFS_SUPPORT
        mops = mount_findfs(g_bdfsmap, filesystemtype);
#endif
        if (mops == NULL) {
            // ferr("ERROR: Failed to find block based file system %s\n",
            //     filesystemtype);
            ret = -ENODEV;
            goto errout_with_inode;
        }
    } else if (source != NULL &&
            (ret = find_mtddriver(source, &drvr_inode)) >= 0)
    {
        /* Find the MTD based file system */

#ifdef MDFS_SUPPORT
        mops = mount_findfs(g_mdfsmap, filesystemtype);
#endif
        if (mops == NULL) {
            // ferr("ERROR: Failed to find MTD based file system %s\n",
            //      filesystemtype);
            ret = -ENODEV;
            goto errout_with_inode;
        }
    } else
#ifdef NODFS_SUPPORT
    if ((mops = mount_findfs(g_nonbdfsmap, filesystemtype)) != NULL) {
    } else
#endif
    {
        ferr("ERROR: Failed to find block driver %s\n", source);
        ret = -ENOTBLK;
        goto errout;
    }

    ret = inode_semtake();
    if (ret < 0) {
        goto errout_with_inode;
    }

    /* Check if the inode already exists */
    SETUP_SEARCH(&desc, target, false);

    ret = inode_find(&desc);
    if (ret >= 0) {
        /* Successfully found.  The reference count on the inode has been
        * incremented.
        */
        mountpt_inode = desc.node;

        /* But is it a directory node (i.e., not a driver or other special
        * node)?
        */

        if (!INODE_IS_PSEUDODIR(mountpt_inode)) {
            // ferr("ERROR: target %s exists and is a special node\n", target);
            ret = -ENOTDIR;
            inode_release(mountpt_inode);
            goto errout_with_semaphore;
        }
    } else {
        /* Insert a dummy node -- we need to hold the inode semaphore
        * to do this because we will have a momentarily bad structure.
        * NOTE that the new inode will be created with an initial reference
        * count of zero.
        */
        ret = inode_reserve(target, 0777, &mountpt_inode);
        if (ret < 0) {
          /* inode_reserve can fail for a couple of reasons, but the most
           * likely one is that the inode already exists. inode_reserve may
           * return:
           *
           *  -EINVAL - 'path' is invalid for this operation
           *  -EEXIST - An inode already exists at 'path'
           *  -ENOMEM - Failed to allocate in-memory resources for the
           *            operation
           */

            // ferr("ERROR: Failed to reserve inode for target %s\n", target);
            goto errout_with_semaphore;
        }
    }

    /* Bind the block driver to an instance of the file system.  The file
    * system returns a reference to some opaque, fs-dependent structure
    * that encapsulates this binding.
    */

    if (mops->bind == NULL) {
        /* The filesystem does not support the bind operation ??? */
        // ferr("ERROR: Filesystem does not support bind\n");
        ret = -EINVAL;
        goto errout_with_mountpt;
    }

    /* Increment reference count for the reference we pass to the file system */

#if defined(BDFS_SUPPORT) || defined(MDFS_SUPPORT)
#ifdef NODFS_SUPPORT
    if (drvr_inode != NULL)
#endif
    {
        drvr_inode->i_crefs++;
    }
#endif

    /* On failure, the bind method returns -errorcode */

#if defined(BDFS_SUPPORT) || defined(MDFS_SUPPORT)
    ret = mops->bind(drvr_inode, data, &fshandle);
#else
    ret = mops->bind(NULL, data, &fshandle);
#endif
    if (ret < 0) {
        /* The inode is unhappy with the driver for some reason.  Back out
        * the count for the reference we failed to pass and exit with an
        * error.
        */

        // ferr("ERROR: Bind method failed: %d\n", ret);

#if defined(BDFS_SUPPORT) || defined(MDFS_SUPPORT)
#ifdef NODFS_SUPPORT
        if (drvr_inode != NULL)
#endif
        {
            drvr_inode->i_crefs--;
        }
#endif

        goto errout_with_mountpt;
    }

    /* We have it, now populate it with driver specific information. */
    INODE_SET_MOUNTPT(mountpt_inode);

    mountpt_inode->u.i_mops  = mops;
    mountpt_inode->i_private = fshandle;
    inode_semgive();

    /* We can release our reference to the blkdrver_inode, if the filesystem
    * wants to retain the blockdriver inode (which it should), then it must
    * have called inode_addref().  There is one reference on mountpt_inode
    * that will persist until umount2() is called.
    */

#if defined(BDFS_SUPPORT) || defined(MDFS_SUPPORT)
#ifdef NODFS_SUPPORT
    if (drvr_inode != NULL)
#endif
    {
        inode_release(drvr_inode);
    }
#endif

    RELEASE_SEARCH(&desc);
    return 0; /* OK */

errout_with_mountpt:
    inode_release(mountpt_inode);
    inode_remove(target);

errout_with_semaphore:
    inode_semgive();
    RELEASE_SEARCH(&desc);

errout_with_inode:

#if defined(BDFS_SUPPORT) || defined(MDFS_SUPPORT)
    if (drvr_inode != NULL) {
        inode_release(drvr_inode);
    }
#endif

errout:
    return ret;

#else
    // ferr("ERROR: No filesystems enabled\n");
    return -ENOSYS;
#endif /* BDFS_SUPPORT || MDFS_SUPPORT || NODFS_SUPPORT */
}

int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data)
{
    int ret;

    ret = nx_mount(source, target, filesystemtype, mountflags, data);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1; /* ERROR */
    }

    return ret;
}

