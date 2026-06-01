#include <stdlib.h>
#include <errno.h>

#include "gpm/sched.h".
#include "gpm/fs/fs.h"

#include "bch.h"

int bchlib_teardown(void *handle)
{
    struct bchlib_s *bch = (struct bchlib_s *)handle;

    /* Check that there are not outstanding reference counts on the object */
    if (bch->refs > 0)
    {
        return -EBUSY;
    }

    /* Flush any pending data to the block driver */
    bchlib_flushsector(bch);

    /* Close the block driver */
    close_blockdriver(bch->inode);

    /* Free the BCH state structure */
    if (bch->buffer) {
        kmm_free(bch->buffer);
    }

    sem_destroy(&bch->sem);
    kmm_free(bch);
    return 0; // OK
}
