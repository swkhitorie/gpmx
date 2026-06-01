#include <stdbool.h>
#include <errno.h>

#include "gpm/fs/fs.h"
#include "gpm/drivers/drivers.h"

#include "bch.h"

int bchdev_register(const char *blkdev, const char *chardev,
                    bool readonly)
{
    void *handle;
    int ret;

    // finfo("blkdev=\"%s\" chardev=\"%s\" readonly=%c\n",
    //         blkdev, chardev, readonly ? 'T' : 'F');

    /* Setup the BCH lib functions */

    ret = bchlib_setup(blkdev, readonly, &handle);
    if (ret < 0) {
        // ferr("ERROR: bchlib_setup failed: %d\n", -ret);
        return ret;
    }

    /* Then setup the character device */
    ret = register_driver(chardev, &bch_fops, 0666, handle);
    if (ret < 0) {
        // ferr("ERROR: register_driver failed: %d\n", -ret);
        bchlib_teardown(handle);
        handle = NULL;
    }

    return ret;
}
