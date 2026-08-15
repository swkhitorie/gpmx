#include <gpmx/config.h>

#include "romfs_test.h"
#include "gromfs.h"
#include <mlog.h>

struct dfs_file file;
struct dfs_fstat stata;
uint8_t databuf[64];
uint32_t offset = 0;

int romfs_test(int argc, char **argv)
{
    int ret = dfs_romfs_open(&file, "/test_file1.txt");
    if (ret) {
        KMRAW("cannot find /test_file1.txt\r\n");
        return -1;
    } else {
        dfs_romfs_stat(&file, &stata);

        KMRAW("find /test_file1.txt, size: %db\r\n", stata.st_size);
        for (offset = 0; offset < stata.st_size;) {
            int sz = dfs_romfs_read(&file, databuf, 1, &offset);
            if (sz == 1) {
                KMRAW("%c", databuf[0]);
            }
        }

        dfs_romfs_close(&file);
    }

    ret = dfs_romfs_open(&file, "/test_file2.txt");
    if (ret) {
        KMRAW("cannot find /test_file2.txt\r\n");
        return -1;
    } else {
        dfs_romfs_stat(&file, &stata);

        KMRAW("find /test_file2.txt, size: %db\r\n", stata.st_size);
        for (offset = 0; offset < stata.st_size;) {
            int sz = dfs_romfs_read(&file, databuf, 1, &offset);
            if (sz == 1) {
                KMRAW("%c", databuf[0]);
            }
        }

        dfs_romfs_close(&file);
    }

    return 0;
}
