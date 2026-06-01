#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <poll.h>

#include "gpm/fs/fs.h"
#include "nuttx/fs/ioctl.h"
#include "gpm/drivers/drivers.h"

#include "bch.h"

static int     bch_open(struct file *filep);
static int     bch_close(struct file *filep);
static off_t   bch_seek(struct file *filep, off_t offset, int whence);
static ssize_t bch_read(struct file *filep, char *buffer,
                 size_t buflen);
static ssize_t bch_write(struct file *filep, const char *buffer,
                 size_t buflen);
static int     bch_ioctl(struct file *filep, int cmd,
                 unsigned long arg);
static int     bch_poll(struct file *filep, struct pollfd *fds,
                 bool setup);
static int     bch_unlink(struct inode *inode);

const struct file_operations bch_fops =
{
  bch_open,    /* open */
  bch_close,   /* close */
  bch_read,    /* read */
  bch_write,   /* write */
  bch_seek,    /* seek */
  bch_ioctl,   /* ioctl */
  bch_poll,     /* poll */
  bch_unlink /* unlink */
};


static int bch_poll(struct file *filep, struct pollfd *fds,
                    bool setup)
{
    if (setup) {
        fds->revents |= (fds->events & (POLLIN | POLLOUT));
        if (fds->revents != 0) {
            nxsem_post(fds->sem);
        }
    }

    return 0; // OK
}

static int bch_open(struct file *filep)
{
    struct inode *inode = filep->f_inode;
    struct bchlib_s *bch;
    int ret = 0; // OK

    bch = (struct bchlib_s *)inode->i_private;

    /* Increment the reference count */
    ret = bchlib_semtake(bch);
    if (ret < 0) {
        return ret;
    }

    if (bch->refs == MAX_OPENCNT) {
        ret = -EMFILE;
    } else {
        bch->refs++;
    }

    bchlib_semgive(bch);
    return ret;
}

static int bch_close(struct file *filep)
{
    struct inode *inode = filep->f_inode;
    struct bchlib_s *bch;
    int ret = 0; // OK

    bch = (struct bchlib_s *)inode->i_private;

    /* Get exclusive access */
    ret = bchlib_semtake(bch);
    if (ret < 0) {
        return ret;
    }

    /* Flush any dirty pages remaining in the cache */
    bchlib_flushsector(bch);

    /* Decrement the reference count (I don't use bchlib_decref() because I
    * want the entire close operation to be atomic wrt other driver
    * operations.
    */

    if (bch->refs == 0) {
        ret = -EIO;
    } else {
        bch->refs--;
        /* If the reference count decremented to zero AND if the character
        * driver has been unlinked, then teardown the BCH device now.
        */

        if (bch->refs == 0 && bch->unlinked) {
            /* Tear the driver down now. */
            ret = bchlib_teardown((void *)bch);

            /* bchlib_teardown() would only fail if there are outstanding
            * references on the device.  Since we know that is not true, it
            * should not fail at all.
            */

            if (ret >= 0) {
                /* Return without releasing the stale semaphore */
                return 0; // OK
            }
        }
    }

    bchlib_semgive(bch);
    return ret;
}

static off_t bch_seek(struct file *filep, off_t offset, int whence)
{
    struct inode *inode = filep->f_inode;
    struct bchlib_s *bch;
    off_t newpos;
    off_t ret;

    bch = (struct bchlib_s *)inode->i_private;
    ret = bchlib_semtake(bch);
    if (ret < 0) {
        return ret;
    }

    /* Determine the new, requested file position */

    switch (whence) {
    case SEEK_CUR:
        newpos = filep->f_pos + offset;
        break;

    case SEEK_SET:
        newpos = offset;
        break;

    case SEEK_END:
        newpos = bch->sectsize * bch->nsectors + offset;
        break;

    default:
        bchlib_semgive(bch);
        return -EINVAL;
    }

  /* Opengroup.org:
   *
   *  "The lseek() function shall allow the file offset to be set beyond the
   *   end of the existing data in the file. If data is later written at this
   *   point, subsequent reads of data in the gap shall return bytes with the
   *   value 0 until data is actually written into the gap."
   *
   * We can conform to the first part, but not the second.  But return EINVAL
   * if:
   *
   *  "...the resulting file offset would be negative for a regular file,
   *  block special file, or directory."
   */

    if (newpos >= 0) {
        filep->f_pos = newpos;
        ret = newpos;
    } else {
        ret = -EINVAL;
    }

    bchlib_semgive(bch);
    return ret;
}

static ssize_t bch_read(struct file *filep, char *buffer, size_t len)
{
    struct inode *inode = filep->f_inode;
    struct bchlib_s *bch;
    ssize_t ret;

    bch = (struct bchlib_s *)inode->i_private;

    ret = bchlib_semtake(bch);
    if (ret < 0) {
        return ret;
    }

    ret = bchlib_read(bch, buffer, filep->f_pos, len);
    if (ret > 0) {
        filep->f_pos += ret;
    }

    bchlib_semgive(bch);
    return ret;
}

static ssize_t bch_write(struct file *filep, const char *buffer,
                        size_t len)
{
    struct inode *inode = filep->f_inode;
    struct bchlib_s *bch;
    ssize_t ret = -EACCES;

    bch = (struct bchlib_s *)inode->i_private;

    if (!bch->readonly) {
        ret = bchlib_semtake(bch);
        if (ret < 0) {
            return ret;
        }

        ret = bchlib_write(bch, buffer, filep->f_pos, len);
        if (ret > 0) {
            filep->f_pos += ret;
        }

        bchlib_semgive(bch);
    }

    return ret;
}

static int bch_ioctl(struct file *filep, int cmd, unsigned long arg)
{
    struct inode *inode = filep->f_inode;
    struct bchlib_s *bch;
    int ret = -ENOTTY;

    bch = (struct bchlib_s *)inode->i_private;

    switch (cmd) {
      /* This isa request to get the private data structure */

    case DIOC_GETPRIV:
        {
            struct bchlib_s **bchr =
                (struct bchlib_s **)((uintptr_t)arg);

            ret = bchlib_semtake(bch);
            if (ret < 0) {
                return ret;
            }

            if (!bchr || bch->refs == MAX_OPENCNT) {
                ret   = -EINVAL;
            } else {
                bch->refs++;
                *bchr = bch;
                ret   = 0; // OK
            }

            bchlib_semgive(bch);
        }
        break;

        /* This is a required to return the geometry of the underlying block
        * driver.
        */

    case BIOC_GEOMETRY:
        {
            struct geometry *geo = (struct geometry *)((uintptr_t)arg);
            ret = bch->inode->u.i_bops->geometry(bch->inode, geo);
            if (ret < 0) {
                // ferr("ERROR: geometry failed: %d\n", -ret);
            } else if (!geo->geo_available) {
                // ferr("ERROR: geometry failed: %d\n", -ret);
                ret = -ENODEV;
            }
        }
        break;

    case BIOC_FLUSH:
        {
            /* Flush any dirty pages remaining in the cache */
            ret = bchlib_flushsector(bch);
        }
        break;

        /* Otherwise, pass the IOCTL command on to the contained block
        * driver.
        */

    default:
        {
            struct inode *bchinode = bch->inode;

            /* Does the block driver support the ioctl method? */
            if (bchinode->u.i_bops->ioctl != NULL) {
                ret = bchinode->u.i_bops->ioctl(bchinode, cmd, arg);
            }
        }
        break;
    }

    return ret;
}


static int bch_unlink(struct inode *inode)
{
    struct bchlib_s *bch;
    int ret = 0; // OK

    bch = (struct bchlib_s *)inode->i_private;

    /* Get exclusive access to the BCH device */
    ret = bchlib_semtake(bch);
    if (ret < 0) {
        return ret;
    }

    /* Indicate that the driver has been unlinked */
    bch->unlinked = true;

    /* If there are no open references to the driver then teardown the BCH
    * device now.
    */

    if (bch->refs == 0) {
        /* Tear the driver down now. */
        ret = bchlib_teardown((void *)bch);

        /* bchlib_teardown() would only fail if there are outstanding
        * references on the device.  Since we know that is not true, it
        * should not fail at all.
        */

        if (ret >= 0) {
            /* Return without releasing the stale semaphore */
            return 0; // OK
        }
    }

    bchlib_semgive(bch);
    return ret;
}
