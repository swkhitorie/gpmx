
#include "w25qxx_driver.h"

struct lfs_config *w25qxx_ops = NULL;
static lfs_t lfs_sflash;

int lfs_sflash_init(int nbus)
{
    uint32_t id = 0;
    int ret = 0;

    if (!w25qxx_init(nbus)) {
        return -1;
    }

    w25qxx_ops = w25qxx_get_lfs_ops();
    if (!w25qxx_ops) {
        return -2;
    }

    id = w25qxx_readid();
    if (id != W25Q64_FLASHID) {
        return -3;
    }

	ret = lfs_mount(&lfs_sflash, w25qxx_ops);
	if (ret)  {
        lfs_format(&lfs_sflash, w25qxx_ops);
        lfs_mount(&lfs_sflash, w25qxx_ops);
        return -4;
    }

    return 0;
}

lfs_file_t test_file;
uint32_t boot_count = 0;
int lfs_sflasg_rwtest()
{
    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs_sflash, &test_file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs_sflash, &test_file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs_sflash, &test_file);
    lfs_file_write(&lfs_sflash, &test_file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs_sflash, &test_file);

    // release any resources we were using
    lfs_unmount(&lfs_sflash);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
    return 0;
}
