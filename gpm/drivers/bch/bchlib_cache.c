#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>

#include "bch.h"

int bchlib_flushsector(struct bchlib_s *bch)
{
    struct inode *inode;
    ssize_t ret = 0; // OK

    /* Check if the sector has been modified and is out of synch with the
    * media.
    */

    if (bch->dirty) {
        inode = bch->inode;

        /* Write the sector to the media */

        ret = inode->u.i_bops->write(inode, bch->buffer, bch->sector, 1);
        if (ret < 0) {
            // ferr("Write failed: %zd\n", ret);
            return (int)ret;
        }

        /* The sector is now in sync with the media */
        bch->dirty = false;
    }

    return (int)ret;
}

int bchlib_readsector(struct bchlib_s *bch, size_t sector)
{
    struct inode *inode;
    ssize_t ret = 0; // OK

    if (bch->sector != sector) {
        inode = bch->inode;

        ret = bchlib_flushsector(bch);
        if (ret < 0) {
            //   ferr("Flush failed: %zd\n", ret);
            //   return (int)ret;
        }

        bch->sector = (size_t)-1;

        ret = inode->u.i_bops->read(inode, bch->buffer, sector, 1);
        if (ret < 0) {
            // ferr("Read failed: %zd\n", ret);
            return (int)ret;
        }

        bch->sector = sector;
    }

    return (int)ret;
}
