#include "romfs_test.h"
#include "gromfs.h"

#include <board_config.h>
#define TEST_PRINTF(...)    board_printf(__VA_ARGS__)

struct dfs_file file;
struct dfs_fstat stat;
uint8_t databuf[64];
uint32_t offset = 0;

int romfs_test()
{
    int ret = dfs_romfs_open(&file, "/test_file1.txt");
    if (ret) {
        board_printf("cannot find /test_file1.txt\r\n");
        return -1;
    } else {
        dfs_romfs_stat(&file, &stat);

        board_printf("find /test_file1.txt, size: %db\r\n", stat.st_size);
        for (offset = 0; offset < stat.st_size;) {
            int sz = dfs_romfs_read(&file, databuf, 1, &offset);
            if (sz == 1) {
                TEST_PRINTF("%c", databuf[0]);
            }
        }

        dfs_romfs_close(&file);
    }

    ret = dfs_romfs_open(&file, "/test_file2.txt");
    if (ret) {
        board_printf("cannot find /test_file2.txt\r\n");
        return -1;
    } else {
        dfs_romfs_stat(&file, &stat);

        board_printf("find /test_file2.txt, size: %db\r\n", stat.st_size);
        for (offset = 0; offset < stat.st_size;) {
            int sz = dfs_romfs_read(&file, databuf, 1, &offset);
            if (sz == 1) {
                TEST_PRINTF("%c", databuf[0]);
            }
        }

        dfs_romfs_close(&file);
    }

    return 0;
}
