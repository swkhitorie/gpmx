#include "lfs_test.h"
#include <lfs.h>
#include <lfs_drv.h>

int lfs_test(int argc, char **argv)
{
    lfs_file_t file;
    int ret = 0;
    uint32_t boot_count = 0;
    ret = lfs_file_open(lfs_objget(0), &file, "lfs_test_cnt", LFS_O_RDWR | LFS_O_CREAT);
    if (ret != LFS_ERR_OK) {
        TEST_PRINTF("open file lfs_test_cnt failed: %d\r\n", ret);
        return -1;
    }

    ret = lfs_file_read(lfs_objget(0), &file, &boot_count, sizeof(boot_count));
    if (ret != sizeof(boot_count)) {
        TEST_PRINTF("read size %d, failed: %d\r\n", sizeof(boot_count), ret);
        return -1;
    }

    // update boot count
    boot_count += 1;
    lfs_file_rewind(lfs_objget(0), &file);

    ret = lfs_file_write(lfs_objget(0), &file, &boot_count, sizeof(boot_count));
    if (ret != sizeof(boot_count)) {
        TEST_PRINTF("write size %d, failed: %d\r\n", sizeof(boot_count), ret);
        return -1;
    }

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(lfs_objget(0), &file);

    // release any resources we were using
    lfs_unmount(lfs_objget(0));

    // print the boot count
    TEST_PRINTF("lfs_test_cnt: %d\n", boot_count);

    return 0;
}
