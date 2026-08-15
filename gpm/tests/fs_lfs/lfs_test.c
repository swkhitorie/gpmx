#include <gpmx/config.h>

#include "lfs_test.h"
#include <lfs.h>

#include <mlog.h>

int lfs_test(int argc, char **argv)
{
    lfs_file_t file;
    lfs_t *test_lfs = lfs_objget(0);

    int ret = 0;
    uint32_t boot_count = 0;
    ret = lfs_file_open(test_lfs, &file, "lfs_test_cnt", LFS_O_RDWR | LFS_O_CREAT);
    if (ret != LFS_ERR_OK) {
        KMRAW("open file lfs_test_cnt failed: %d\r\n", ret);
        return -1;
    }

    ret = lfs_file_read(test_lfs, &file, &boot_count, sizeof(boot_count));
    if (ret != sizeof(boot_count)) {
        KMRAW("read size %d, failed: %d\r\n", sizeof(boot_count), ret);
        return -1;
    }

    // update boot count
    boot_count += 1;
    lfs_file_rewind(test_lfs, &file);

    ret = lfs_file_write(test_lfs, &file, &boot_count, sizeof(boot_count));
    if (ret != sizeof(boot_count)) {
        KMRAW("write size %d, failed: %d\r\n", sizeof(boot_count), ret);
        return -1;
    }

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(test_lfs, &file);

    // release any resources we were using
    lfs_unmount(test_lfs);

    // print the boot count
    KMRAW("lfs_test_cnt: %d\n", boot_count);

    return 0;
}
