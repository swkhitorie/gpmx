#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "inode/inode.h"

static int _inode_compare(const char *fname, struct inode *node);
static int _inode_search(struct inode_search_s *desc);
static const char *_inode_getcwd(void);

struct inode *g_root_inode = NULL;

static int _inode_compare(const char *fname, struct inode *node)
{
    char *nname = node->i_name;
    if (!nname) {
        return 1;
    }

    if (!fname) {
        return -1;
    }

    for (;;) {
        /* At the end of the node name? */
        if (!*nname) {
            /* Yes.. also at the end of find name? */
            if (!*fname || *fname == '/') {
                /* Yes.. return match */
                return 0;
            } else {
                /* No... return find name > node name */
                return 1;
            }
        /* At end of the find name? */
        } else if (!*fname || *fname == '/') {
            /* Yes... return find name < node name */
            return -1;
        /* Check for non-matching characters */
        } else if (*fname > *nname) {
            return 1;
        } else if (*fname < *nname) {
            return -1;
        } else {
            /* Not at the end of either string and all of the
            * characters still match.  keep looking.
            */
            fname++;
            nname++;
        }
    }
}

/****************************************************************************
 * Name: _inode_search
 *
 * Description:
 *   Find the inode associated with 'path' returning the inode references
 *   and references to its companion nodes.  This is the internal, common
 *   implementation of inode_search().
 *
 *   If a mountpoint is encountered in the search prior to encountering the
 *   terminal node, the search will terminate at the mountpoint inode.  That
 *   inode and the relative path from the mountpoint, 'relpath' will be
 *   returned.
 *
 *   If a soft link is encountered that is not the terminal node in the path,
 *   that link WILL be deferenced unconditionally.
 *
 * Assumptions:
 *   The caller holds the g_inode_sem semaphore
 *
 ****************************************************************************/
static int _inode_search(struct inode_search_s *desc)
{
    const char   *name;
    struct inode *node    = g_root_inode;
    struct inode *left    = NULL;
    struct inode *above   = NULL;
    const char   *relpath = NULL;
    int ret = -ENOENT;

    /* Get the search path, skipping over the leading '/'.  The leading '/' is
    * mandatory because only absolute paths are expected in this context.
    */
    name  = desc->path;
    if (*name != '/') {
        return -EINVAL;
    }

    /* Traverse the pseudo file system node tree until either (1) all nodes
    * have been examined without finding the matching node, or (2) the
    * matching node is found.
    */
    while (node != NULL) {
        int result = _inode_compare(name, node);

        /* Case 1:  The name is less than the name of the node.
        * Since the names are ordered, these means that there
        * is no peer node with this name and that there can be
        * no match in the filesystem.
        */
        if (result < 0) {
            node = NULL;
            break;

        /* Case 2: the name is greater than the name of the node.
        * In this case, the name may still be in the list to the
        * "right"
        */
        } else if (result > 0) {
            /* Continue looking to the "right" of this inode. */
            left = node;
            node = node->i_peer;

        /* The names match */
        } else {

            /* Now there are three remaining possibilities:
            *   (1) This is the node that we are looking for.
            *   (2) The node we are looking for is "below" this one.
            *   (3) This node is a mountpoint and will absorb all requests
            *       below this one
            */
            name = inode_nextname(name);
            if (*name == '\0' || INODE_IS_MOUNTPT(node)) {
                relpath = name;
                ret = 0;
                break;
            } else {
                above = node;
                left  = NULL;
                node  = node->i_child;
            }
        }
    }

    desc->path    = name;
    desc->node    = node;
    desc->peer    = left;
    desc->parent  = above;
    desc->relpath = relpath;
    return ret;
}

static const char *_inode_getcwd(void)
{
    const char *pwd = "/";
    return pwd;
}

static int local_vasprintf(char **strp, const char *fmt, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0) { 
        *strp = NULL;
        return -1; 
    }

    char *buf = kmm_malloc((size_t)len + 1);

    if (!buf) { 
        *strp = NULL;
        return -1; 
    }

    vsnprintf(buf, (size_t)len + 1, fmt, ap);
    *strp = buf;
    return len;
}

static int local_asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = local_vasprintf(strp, fmt, ap);
    va_end(ap);
    return ret;
}

int inode_search(struct inode_search_s *desc)
{
    int ret;

    if (*desc->path != '/') {
        local_asprintf(&desc->buffer, "%s/%s", _inode_getcwd(), desc->path);
        if (desc->buffer == NULL) {
            return -ENOMEM;
        }

        desc->path = desc->buffer;
    }

    ret = _inode_search(desc);
    return ret;
}

/*
 *   Given a path with node names separated by '/', return the next path
 *   segment name.
*/
const char *inode_nextname(const char *name)
{
    /* Search for the '/' delimiter or the NUL terminator at the end of the
    * path segment.
    */
    while (*name != '\0' && *name != '/') {
        name++;
    }

    /* If we found the '/' delimiter, then the path segment we want begins at
    * the next character (which might also be the NUL terminator).
    */
    while (*name == '/') {
        name++;
    }

    return name;
}
