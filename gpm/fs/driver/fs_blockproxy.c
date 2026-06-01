
#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <semaphore.h>
#include "gpm/fs/fs.h"
#include "gpm/sched.h"
#include "gpm/drivers/drivers.h"

static uint32_t g_devno;

// static sem_t g_devno_sem = SEM_INITIALIZER(1);
#if defined(CONFIG_RTTNANO_ENABLE)
static sem_t g_devno_sem = {.refcount = 254};
#elif defined(CONFIG_FREERTOS_ENABLE)
static sem_t g_devno_sem = {.val = 254};
#endif

char *local_strdup(const char *s)
{
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s) + 1;

    char *dup = (char *)kmm_malloc(len);

    if (dup != NULL) {
        memcpy(dup, s, len);
    }
    return dup;
}

static char *unique_chardev(void)
{
    struct stat statbuf;
    char devbuf[16];
    uint32_t devno;
    int ret;

#if defined(CONFIG_RTTNANO_ENABLE)
    if (g_devno_sem.refcount == 254) {
        sem_init(&g_devno_sem, 0, 1);
    }
#elif defined(CONFIG_FREERTOS_ENABLE)
    if (g_devno_sem.val == 254) {
        sem_init(&g_devno_sem, 0, 1);
    }
#endif

    /* Loop until we get a unique device name */
    for (;;) {

        /* Get the semaphore protecting the path number */
        ret = sem_wait(&g_devno_sem); //nxsem_wait_uninterruptible(&g_devno_sem);
        if (ret < 0) {
            // ferr("ERROR: nxsem_wait_uninterruptible failed: %d\n", ret);
            return NULL;
        }

        /* Get the next device number and release the semaphore */
        devno = ++g_devno;
        sem_post(&g_devno_sem);

        /* Construct the full device number */
        devno &= 0xffffff;
        snprintf(devbuf, 16, "/dev/tmpc%06lx", (unsigned long)devno);

        /* Make sure that file name is not in use */
        ret = nx_stat(devbuf, &statbuf, 1);
        if (ret < 0) {
            extern char *local_strdup(const char *s);
            return local_strdup(devbuf);
        }

        /* It is in use, try again */
    }
}

int block_proxy(struct file *filep, const char *blkdev, int oflags)
{
    char *chardev;
    bool readonly;
    int ret;

    chardev = unique_chardev();
    if (chardev == NULL) {
        // ferr("ERROR: Failed to create temporary device name\n");
        return -ENOMEM;
    }

    /* Should this character driver be read-only? */
    readonly = ((oflags & O_WROK) == 0);

    /* Wrap the block driver with an instance of the BCH driver */
    ret = bchdev_register(blkdev, chardev, readonly);
    if (ret < 0) {
        // ferr("ERROR: bchdev_register(%s, %s) failed: %d\n",
        //     blkdev, chardev, ret);
        goto errout_with_chardev;
    }

    /* Open the newly created character driver */
    oflags &= ~(O_CREAT | O_EXCL | O_APPEND | O_TRUNC);
    ret = file_open(filep, chardev, oflags);
    if (ret < 0) {
        // ferr("ERROR: Failed to open %s: %d\n", chardev, ret);
        goto errout_with_bchdev;
    }

    /* Unlink the character device name.  The driver instance will persist,
    * provided that CONFIG_DISABLE_PSEUDOFS_OPERATIONS=y (otherwise, we have
    * a problem here!)
    */

    ret = nx_unlink(chardev);
    if (ret < 0) {
        // ferr("ERROR: Failed to unlink %s: %d\n", chardev, ret);
        goto errout_with_chardev;
    }

    /* Free the allocated character driver name. */
    kmm_free(chardev);
    return 0; // OK

errout_with_bchdev:
    nx_unlink(chardev);

errout_with_chardev:
    kmm_free(chardev);
    return ret;
}

