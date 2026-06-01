#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <semaphore.h>
#include "gpm/mtd/mtd.h"
#include "gpm/sched.h"

#include "driver/driver.h"

static uint32_t g_devno;
#if defined(CONFIG_RTTNANO_ENABLE)
static sem_t g_devno_sem = {.refcount = 254};
#elif defined(CONFIG_FREERTOS_ENABLE)
static sem_t g_devno_sem = {.val = 254};
#endif

static char *unique_blkdev(void)
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

    for (;;) {

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
        snprintf(devbuf, 16, "/dev/tmpb%06lx", (unsigned long)devno);

        /* Make sure that file name is not in use */
        ret = nx_stat(devbuf, &statbuf, 1);
        if (ret < 0) {
            extern char *local_strdup(const char *s);
            return local_strdup(devbuf);
        }
    }
}

int mtd_proxy(const char *mtddev, int mountflags,
            struct inode **ppinode)
{
    struct inode *mtd;
    char *blkdev;
    int ret;

    blkdev = unique_blkdev();
    if (blkdev == NULL) {
        return -ENOMEM;
    }

    ret = find_mtddriver(mtddev, &mtd);
    if (ret < 0) {
        goto out_with_blkdev;
    }

    ret = ftl_initialize_by_path(blkdev, mtd->u.i_mtd);
    inode_release(mtd);
    if (ret < 0) {
        goto out_with_blkdev;
    }

    /* Open the newly created block driver */
    ret = open_blockdriver(blkdev, mountflags, ppinode);
    if (ret < 0) {
        goto out_with_fltdev;
    }

out_with_fltdev:
    nx_unlink(blkdev);
out_with_blkdev:
    kmm_free(blkdev);
    return ret;
}
