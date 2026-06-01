#include <sys/types.h>
#include <sys/mount.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#include <gpm/sched.h>
#include <gpm/fs/fs.h>

#include "bch.h"

int bchlib_setup(const char *blkdev, bool readonly, void **handle)
{
    struct bchlib_s *bch;
    struct geometry geo;
    int ret;

    bch = (struct bchlib_s *)kmm_zalloc(sizeof(struct bchlib_s));
    if (!bch) {
        // ferr("ERROR: Failed to allocate BCH structure\n");
        return -ENOMEM;
    }

    ret = open_blockdriver(blkdev, readonly ? MS_RDONLY : 0, &bch->inode);
    if (ret < 0) {
        // ferr("ERROR: Failed to open driver %s: %d\n", blkdev, -ret);
        goto errout_with_bch;
    }

    ret = bch->inode->u.i_bops->geometry(bch->inode, &geo);
    if (ret < 0) {
        // ferr("ERROR: geometry failed: %d\n", -ret);
        goto errout_with_bch;
    }

    if (!geo.geo_available) {
        // ferr("ERROR: geometry failed: %d\n", -ret);
        ret = -ENODEV;
        goto errout_with_bch;
    }

    if (!readonly && (!bch->inode->u.i_bops->write || !geo.geo_writeenabled)) {
        // ferr("ERROR: write access not supported\n");
        ret = -EACCES;
        goto errout_with_bch;
    }

    sem_init(&bch->sem, 0, 1);
    bch->nsectors = geo.geo_nsectors;
    bch->sectsize = geo.geo_sectorsize;
    bch->sector   = (size_t)-1;
    bch->readonly = readonly;

    /* Allocate the sector I/O buffer */
    bch->buffer = kmm_malloc(bch->sectsize);

    if (!bch->buffer) {
        // ferr("ERROR: Failed to allocate sector buffer\n");
        ret = -ENOMEM;
        goto errout_with_bch;
    }

    *handle = bch;
    return 0; // OK

errout_with_bch:
    kmm_free(bch);
    return ret;
}
