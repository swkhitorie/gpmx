#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>

#include <nuttx/fs/fs.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/drivers/drivers.h>

#include "bch.h"


int bchdev_unregister(const char *chardev)
{
    struct bchlib_s *bch;
    struct file filestruct;
    int ret;

    /* Open the character driver associated with chardev */

    ret = file_open(&filestruct, chardev, O_RDONLY);
    if (ret < 0) {
        // _err("ERROR: Failed to open %s: %d\n", chardev, ret);
        return ret;
    }

    /* Get a reference to the internal data structure.  On success, we
    * will hold a reference count on the state structure.
    */

    ret = file_ioctl(&filestruct, DIOC_GETPRIV,
                    (unsigned long)((uintptr_t)&bch));
    file_close(&filestruct);

    if (ret < 0) {
        _err("ERROR: ioctl failed: %d\n", ret);
        return ret;
    }

    /* Lock out context switches.  If there are no other references
    * and no context switches, then we can assume that we can safely
    * teardown the driver.
    */

    sched_lock();

     /* Check if the internal structure is non-busy (we hold one reference). */
    if (bch->refs > 1) {
        ret = -EBUSY;
        goto errout_with_lock;
    }

    /* Unregister the driver (this cannot suspend or we lose our non-preemptive
    * state!).  Once the driver is successfully unregistered, we can assume
    * we have exclusive access to the state instance.
    */

    ret = unregister_driver(chardev);
    if (ret < 0) {
        goto errout_with_lock;
    }

    sched_unlock();

    /* Release the internal structure */
    bch->refs = 0;
    return bchlib_teardown(bch);

errout_with_lock:
    bch->refs--;
    sched_unlock();
    return ret;
}
