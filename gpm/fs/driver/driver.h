#ifndef FS_DRIVER_DRIVER_H
#define FS_DRIVER_DRIVER_H

#include "gpm/fs/fs.h"
#include "inode/inode.h"

#if defined(__cplusplus)
extern "C" {
#endif

int find_blockdriver(const char *pathname, int mountflags,
                    struct inode **ppinode);

int block_proxy(struct file *filep, const char *blkdev, int oflags);

int mtd_proxy(const char *mtddev, int mountflags,
            struct inode **ppinode);

int find_mtddriver(const char *pathname, struct inode **ppinode);

#if defined(__cplusplus)
}
#endif

#endif
