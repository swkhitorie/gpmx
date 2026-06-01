#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "bch.h"

ssize_t bchlib_read(void *handle, char *buffer, size_t offset,
                    size_t len)
{
    struct bchlib_s *bch = (struct bchlib_s *)handle;
    size_t   nsectors;
    size_t   sector;
    uint16_t sectoffset;
    size_t   nbytes;
    size_t   bytesread;
    int      ret;

    /* Get rid of this special case right away */

    if (len < 1) {
        return 0;
    }

    /* Convert the file position into a sector number an offset. */
    sector     = offset / bch->sectsize;
    sectoffset = offset - sector * bch->sectsize;

    if (sector >= bch->nsectors) {
        /* Return end-of-file */
        return 0;
    }

    /* Read the initial partial sector */

    bytesread = 0;
    if (sectoffset > 0) {

        /* Read the sector into the sector buffer */
        ret = bchlib_readsector(bch, sector);
        if (ret < 0) {
            return ret;
        }

        /* Copy the tail end of the sector to the user buffer */
        if (sectoffset + len > bch->sectsize) {
            nbytes = bch->sectsize - sectoffset;
        } else {
            nbytes = len;
        }

        memcpy(buffer, &bch->buffer[sectoffset], nbytes);
        /* Adjust pointers and counts */

        sector++;
        if (sector >= bch->nsectors) {
            return nbytes;
        }

        bytesread  = nbytes;
        buffer    += nbytes;
        len       -= nbytes;
    }

    /* Then read all of the full sectors following the partial sector directly
    * into the user buffer.
    */
    if (len >= bch->sectsize) {
        nsectors = len / bch->sectsize;
        if (sector + nsectors > bch->nsectors) {
            nsectors = bch->nsectors - sector;
        }

        ret = bch->inode->u.i_bops->read(bch->inode, (uint8_t *)buffer,
                                        sector, nsectors);
        if (ret < 0) {
            // ferr("ERROR: Read failed: %d\n", ret);
            return ret;
        }

        /* Adjust pointers and counts */
        sector    += nsectors;
        nbytes     = nsectors * bch->sectsize;
        bytesread += nbytes;

        if (sector >= bch->nsectors) {
            return bytesread;
        }

        buffer    += nbytes;
        len       -= nbytes;
    }

  /* Then read any partial final sector */
    if (len > 0) {

        /* Read the sector into the sector buffer */
        ret = bchlib_readsector(bch, sector);
        if (ret < 0) {
            return ret;
        }

        /* Copy the head end of the sector to the user buffer */
        memcpy(buffer, bch->buffer, len);

        /* Adjust counts */
        bytesread += len;
    }

    return bytesread;
}
