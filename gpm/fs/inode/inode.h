#ifndef INCLUDE_INODE_H_
#define INCLUDE_INODE_H_

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include "gpm/fs/fs.h"
#include "gpm/sched.h"

#define SETUP_SEARCH(d,p,n) \
    do { \
        (d)->path     = (p); \
        (d)->node     = NULL; \
        (d)->peer     = NULL; \
        (d)->parent   = NULL; \
        (d)->relpath  = NULL; \
        (d)->buffer   = NULL; \
        (d)->nofollow = (n); \
    } while (0)

#define RELEASE_SEARCH(d) \
    do { \
        if ((d)->buffer != NULL) { \
            kmm_free((d)->buffer); \
            (d)->buffer  = NULL; \
        } \
    } while (0)

struct inode_search_s {
    const char *path;      /* Path of inode to find */
    struct inode *node;    /* Pointer to the inode found */
    struct inode *peer;    /* Node to the "left" for the found inode */
    struct inode *parent;  /* Node "above" the found inode */
    const char *relpath;   /* Relative path into the mountpoint */
    char *buffer;          /* Path expansion buffer */
    bool nofollow;             /* true: Don't follow terminal soft link */
};

typedef int (*foreach_inode_t)( struct inode *node,
                                char dirpath[PATH_MAX],
                                void *arg);

#if defined(__cplusplus)
extern "C" {
#endif

extern struct inode *g_root_inode;

void inode_initialize(void);
int  inode_semtake(void);
void inode_semgive(void);
void inode_root_reserve(void);
int  inode_search(struct inode_search_s *desc);
int  inode_find(struct inode_search_s *desc);
int  inode_getpath(struct inode *node, char *path);
void inode_free(struct inode *node);
const char *inode_nextname(const char *name);
int  inode_remove(const char *path);
int  inode_addref(struct inode *inode);
void inode_release(struct inode *inode);
int  foreach_inode(foreach_inode_t handler, void *arg);
int  inode_reserve(const char *path,
                  mode_t mode, struct inode **inode);
struct inode *inode_unlink(const char *path);

int inode_checkflags(struct inode *inode, int oflags);
int inode_stat(struct inode *inode, struct stat *buf, int resolve);
int inode_chstat(struct inode *inode,
                const struct stat *buf, int flags, int resolve);

int files_allocate(struct inode *inode, int oflags, off_t pos,
                   void *priv, int minfd);

int dir_allocate(struct file *filep, const char *relpath);

#if defined(__cplusplus)
}
#endif

#endif
