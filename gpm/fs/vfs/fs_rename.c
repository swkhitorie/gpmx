/****************************************************************************
 * fs/vfs/fs_rename.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>
#include <errno.h>

#include <gpmx/config.h>
#include <gpm/fs/fs.h>
#include <inode/inode.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#undef FS_HAVE_RENAME
#if !defined(CONFIG_DISABLE_MOUNTPOINT) || !defined(CONFIG_DISABLE_PSEUDOFS_OPERATIONS)
#  define FS_HAVE_RENAME 1
#endif

#ifdef FS_HAVE_RENAME

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: pseudorename
 *
 * Description:
 *   Rename an inode in the pseudo file system
 *
 ****************************************************************************/

#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
static int pseudorename(const char *oldpath, struct inode *oldinode,
                        const char *newpath)
{
    struct inode_search_s newdesc;
    struct inode *newinode;
    char *subdir = NULL;
    int ret;

    /* According to POSIX, any old inode at this path should be removed
    * first, provided that it is not a directory.
    */
next_subdir:

    SETUP_SEARCH(&newdesc, newpath, true);
    ret = inode_find(&newdesc);
    if (ret >= 0) {

        /* We found it.  Get the search results */
        newinode = newdesc.node;
        DEBUGASSERT(newinode != NULL);

        /* If the old and new inodes are the same, then this is an attempt to
        * move the directory entry onto itself.  Let's not but say we did.
        */
        if (oldinode == newinode) {
            inode_release(newinode);
            ret = 0; // OK
            goto errout;
        }

        /* Make sure that the old path does not lie on a mounted volume. */
        if (INODE_IS_MOUNTPT(newinode)) {
            inode_release(newinode);
            ret = -EXDEV;
            goto errout;
        }

        /* We found it and it appears to be a "normal" inode.  Is it a
        * directory (i.e, an operation-less inode or an inode with children)?
        */
        if (newinode->u.i_ops == NULL || newinode->i_child != NULL) {
            char *subdirname;
            char *tmp;

            inode_release(newinode);

            /* Yes.. In this case, the target of the rename must be a
            * subdirectory of newinode, not the newinode itself.  For
            * example: mv b a/ must move b to a/b.
            */
            subdirname = basename((char *)oldpath);
            tmp        = subdir;
            subdir     = NULL;

            subdir = kmm_malloc(PATH_MAX);

            asprintf(&subdir, "%s/%s", newpath, subdirname);

            if (tmp != NULL) {
                kmm_free(tmp);
            }

            if (subdir == NULL) {
                ret = -ENOMEM;
                goto errout;
            }

            sprintf(subdir, "%s/%s", newpath, subdirname);

            newpath = subdir;

            /* This can be a recursive case, another inode may already exist
            * at oldpth/subdirname.  In that case, we need to do this all
            * over again.  A nasty goto is used because I am lazy.
            */

            RELEASE_SEARCH(&newdesc);
            goto next_subdir;
        } else {
            /* Not a directory... remove it.  It may still be something
            * important (like a driver), but we will just have to suffer
            * the consequences.
            *
            * NOTE (1) that we not bother to check the error.  If we
            * failed to remove the inode for some reason, then
            * inode_reserve() will complain below, and (2) the inode
            * won't really be removed until we call inode_release();
            */

            inode_remove(newpath);
        }

        inode_release(newinode);
    }

    /* Create a new, empty inode at the destination location.
    * NOTE that the new inode will be created with a reference count
    * of  zero.
    */

    ret = inode_semtake();
    if (ret < 0) {
        goto errout;
    }

    ret = inode_reserve(newpath, 0777, &newinode);
    if (ret < 0) {
        /* It is an error if a node at newpath already exists in the tree
        * OR if we fail to allocate memory for the new inode (and possibly
        * any new intermediate path segments).
        */

        ret = -EEXIST;
        goto errout_with_sem;
    }

    /* Copy the inode state from the old inode to the newly allocated inode */
    newinode->i_child   = oldinode->i_child;   /* Link to lower level inode */
    newinode->i_flags   = oldinode->i_flags;   /* Flags for inode */
    newinode->u.i_ops   = oldinode->u.i_ops;   /* Inode operations */
    newinode->i_private = oldinode->i_private; /* Per inode driver private data */

    ret = inode_remove(oldpath);
    if (ret < 0 && ret != -EBUSY) {
        /* Remove the new node we just recreated */
        inode_remove(newpath);
        goto errout_with_sem;
    }

    /* Remove all of the children from the unlinked inode */
    oldinode->i_child  = NULL;
    oldinode->i_parent = NULL;
    ret = 0; // OK

errout_with_sem:
    inode_semgive();

errout:
    RELEASE_SEARCH(&newdesc);

    if (subdir != NULL) {
        kmm_free(subdir);
    }

    return ret;
}

#endif /* CONFIG_DISABLE_PSEUDOFS_OPERATIONS */

/****************************************************************************
 * Name: mountptrename
 *
 * Description:
 *   Rename a file residing on a mounted volume.
 *
 ****************************************************************************/

static int mountptrename(const char *oldpath, struct inode *oldinode,
                         const char *oldrelpath, const char *newpath)
{
    struct inode_search_s newdesc;
    struct inode *newinode;
    const char *newrelpath;
    char *subdir = NULL;
    int ret;

    DEBUGASSERT(oldinode->u.i_mops);

    /* If the file system does not support the rename() method, then bail now.
    * As of this writing, only NXFFS does not support the rename method.  A
    * good fallback might be to copy the oldrelpath to the correct location,
    * then unlink it.
    */

    if (oldinode->u.i_mops->rename == NULL) {
        return -ENOSYS;
    }

    /* Get an inode for the new relpath -- it should lie on the same
    * mountpoint
    */
    SETUP_SEARCH(&newdesc, newpath, true);

    ret = inode_find(&newdesc);
    if (ret < 0) {
        /* There is no mountpoint that includes in this path */
        goto errout_with_newsearch;
    }

    /* Get the search results */
    newinode   = newdesc.node;
    newrelpath = newdesc.relpath;
    DEBUGASSERT(newinode != NULL && newrelpath != NULL);

    /* Verify that the two paths lie on the same mountpoint inode */
    if (oldinode != newinode) {
        ret = -EXDEV;
        goto errout_with_newinode;
    }

    /* Does a directory entry already exist at the 'rewrelpath'?  And is it
    * not the same directory entry that we are moving?
    *
    * If the directory entry at the newrelpath is a regular file, then that
    * file should be removed first.
    *
    * If the directory entry at the target is a directory, then the source
    * file should be moved "under" the directory, i.e., if newrelpath is a
    * directory, then rename(b,a) should use move the olrelpath should be
    * moved as if rename(b,a/basename(b)) had been called.
    */

    if (oldinode->u.i_mops->stat != NULL &&
        strcmp(oldrelpath, newrelpath) != 0)
    {
        struct stat buf;

next_subdir:

        /* Something exists for this directory entry.  Do nothing in the
        * degenerate case where a directory or file is being moved to
        * itself.
        */

        if (strcmp(oldrelpath, newrelpath) != 0) {
            ret = oldinode->u.i_mops->stat(oldinode, newrelpath, &buf);
            if (ret >= 0) {
                /* Is the directory entry a directory? */
                if (S_ISDIR(buf.st_mode)) {
                    char *subdirname;

                    /* Yes.. In this case, the target of the rename must be a
                    * subdirectory of newinode, not the newinode itself.  For
                    * example: mv b a/ must move b to a/b.
                    */

                    subdirname = basename((char *)oldrelpath);

                    /* Special case the root directory */
                    if (*newrelpath == '\0') {
                        if (subdir != NULL) {
                            kmm_free(subdir);
                            subdir = NULL;
                        }

                        newrelpath = subdirname;
                    } else {
                        char *tmp = subdir;

                        subdir = NULL;
                        asprintf(&subdir, "%s/%s", newrelpath,
                                subdirname);

                        if (tmp != NULL) {
                            kmm_free(tmp);
                        }

                        if (subdir == NULL) {
                            ret = -ENOMEM;
                            goto errout_with_newinode;
                        }

                        newrelpath = subdir;
                    }

                    /* This can be a recursive, another directory may already
                    * exist at the newrelpath.  In that case, we need to
                    * do this all over again.  A nasty goto is used because
                    * I am lazy.
                    */

                    goto next_subdir;
                } else if (oldinode->u.i_mops->unlink) {
                    /* No.. newrelpath must refer to a regular file.  Attempt
                    * to remove the file before doing the rename.
                    *
                    * NOTE that errors are not handled here.  If we failed to
                    * remove the file, then the file system 'rename' method
                    * should check that.
                    */

                    oldinode->u.i_mops->unlink(oldinode, newrelpath);
                }
            }
        }
    }

    /* Just declare success of the oldrepath and the newrelpath point to
    * the same directory entry.  That directory entry should have been
    * stat'ed above to assure that it exists.
    */

    ret = 0; // OK
    if (strcmp(oldrelpath, newrelpath) != 0) {
        /* Perform the rename operation using the relative paths at the common
        * mountpoint.
        */
        ret = oldinode->u.i_mops->rename(oldinode, oldrelpath, newrelpath);
    }

errout_with_newinode:
    inode_release(newinode);

errout_with_newsearch:
    RELEASE_SEARCH(&newdesc);

    if (subdir != NULL) {
        kmm_free(subdir);
    }

    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rename
 *
 * Description:
 *   Rename a file or directory.
 *
 ****************************************************************************/

int rename(const char *oldpath, const char *newpath)
{
    struct inode_search_s olddesc;
    struct inode *oldinode;
    int ret;

    if (!oldpath || *oldpath == '\0' ||
        !newpath || *newpath == '\0')
    {
        ret = -EINVAL;
        goto errout;
    }

    /* Get an inode that includes the oldpath */
    SETUP_SEARCH(&olddesc, oldpath, true);

    ret = inode_find(&olddesc);
    if (ret < 0) {
        /* There is no inode that includes in this path */
        goto errout_with_oldsearch;
    }

    /* Get the search results */
    oldinode = olddesc.node;
    DEBUGASSERT(oldinode != NULL);

    /* Verify that the old inode is a valid mountpoint. */
    if (INODE_IS_MOUNTPT(oldinode) && *olddesc.relpath != '\0') {
        ret = mountptrename(oldpath, oldinode, olddesc.relpath, newpath);
    } else {
        ret = pseudorename(oldpath, oldinode, newpath);
    }

    inode_release(oldinode);

errout_with_oldsearch:
    RELEASE_SEARCH(&olddesc);

errout:
    if (ret < 0) {
        set_errno(-ret);
        return -1; // ERROR
    }

    return 0; // OK
}

#endif /* FS_HAVE_RENAME */
