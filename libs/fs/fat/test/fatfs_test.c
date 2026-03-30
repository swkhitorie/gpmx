#include "ff.h"
#include "fatfs_test.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// ff.c line 2928 create_name() func:
// memset(dp->fn, ' ', 11) change to memset(dp->fn, ' ', 12) ??

static const char *FR_Table[]=  {
	"FR_OK: (0) Succeeded",
	"FR_DISK_ERR: (1) A hard error occurred in the low level disk I/O layer",
	"FR_INT_ERR: (2) Assertion failed",
	"FR_NOT_READY: (3) The physical drive cannot work",
	"FR_NO_FILE: (4) Could not find the file ",
	"FR_NO_PATH: (5) Could not find the path",
	"FR_INVALID_NAME: (6) The path name format is invalid",
	"FR_DENIED: (7) Access denied due to prohibited access or directory full",
	"FR_EXIST: (8) Access denied due to prohibited access",
	"FR_INVALID_OBJECT: (9) The file/directory object is invalid",
	"FR_WRITE_PROTECTED: (10) The physical drive is write protected",
	"FR_INVALID_DRIVE: (11) The logical drive number is invalid",
	"FR_NOT_ENABLED: (12) The volume has no work area",
	"FR_NO_FILESYSTEM: (13) There is no valid FAT volume",
	"FR_MKFS_ABORTED: (14) The f_mkfs() aborted due to any parameter error",
	"FR_TIMEOUT: (15) Could not get a grant to access the volume within defined period",
	"FR_LOCKED: (16) The operation is rejected according to the file sharing policy",
	"FR_NOT_ENOUGH_CORE: (17) LFN working buffer could not be allocated",
	"FR_TOO_MANY_OPEN_FILES: (18) Number of open files > _FS_SHARE",
	"FR_INVALID_PARAMETER: (19) Given parameter is invalid"
};

static FIL file;
static FIL rd_fp;
static char read_buffer[256];
static uint8_t rd_buf[1024];

static int format_ramdisk(void);
static int ff_ls(const char *path);
static int ff_cat(char *path);
static int  ff_create_and_write(void);
static int  test_read_and_verify(void);
static int  test_file_info(void);

int format_ramdisk(void)
{
#if FF_USE_MKFS
    FRESULT fr;
    BYTE work[FF_MAX_SS]; /* format work zone */
    MKFS_PARM opt;

    opt.fmt = FM_ANY;     /* select FAT type, auto */
    opt.n_fat = 1;        /* one fat table */
    opt.align = 0;        /* align auto */
    opt.n_root = 512;     /* root item number */
    opt.au_size = 0;

    fr = f_mkfs("0:", &opt, work, sizeof(work));
    if (fr != FR_OK) {
        printf("[ERROR] format RAM disk failed: %s \n", FR_Table[fr]);
        return -1;
    }

    printf("[INFO] RAM disk format success \n");
    return 0;
#else
    printf("[WARN] FF_USE_MKFS not enable, jump\n");
    return 0;
#endif
}

int ff_ls(const char *path)
{
	FRESULT result;
    FILINFO fileinfo;
    DIR dirinfo;
	uint32_t cnt = 0;

    memset(&fileinfo, 0, sizeof(FILINFO));
    memset(&dirinfo, 0, sizeof(DIR));

	result = f_opendir(&dirinfo, path);
	if (result != FR_OK) {
		TEST_PRINTF("[fat] open root dir failed %s \r\n", FR_Table[result]);
		return -1;
	}

	TEST_PRINTF("properties  |  size |    name     \r\n");
	for (cnt = 0; ;cnt++) {
		result = f_readdir(&dirinfo, &fileinfo);
		if (result != FR_OK || fileinfo.fname[0] == 0) {
			break;
		}

		if (fileinfo.fname[0] == '.') {
			continue;
		}

		if (fileinfo.fattrib & AM_DIR) {
			TEST_PRINTF("dir(0x%x)\t",(uint32_t)fileinfo.fattrib);
		}else {
			TEST_PRINTF("file(0x%x)\t",(uint32_t)fileinfo.fattrib);
		}

		TEST_PRINTF(" %d \t",(uint32_t)fileinfo.fsize);
		TEST_PRINTF(" %s\r\n",fileinfo.fname);
	}

	return 0;
}

int ff_cat(char *path)
{
	FRESULT ret;
	UINT bw;

	ret = f_open(&rd_fp, path, FA_OPEN_EXISTING | FA_READ);
	if (ret != FR_OK) {
		TEST_PRINTF("[fat] %s open file failed: %s \r\n", path, FR_Table[ret]);
		return -1;
	}

	ret = f_read(&rd_fp, rd_buf, rd_fp.obj.objsize, &bw);
	if (ret != FR_OK) {
		TEST_PRINTF("[fat] %s read file failed %s \r\n", path, FR_Table[ret]);
        return -2;
	}

    TEST_PRINTF("file: %s:\r\n", path);
    TEST_PRINTF("%s", rd_buf);

	f_close(&rd_fp);
	return 0;
}

int ff_create_and_write(void)
{
    FRESULT fr;
    UINT bw;
    const char *test_data = "This is a UnitTest";

    fr = f_open(&file, "0:/fatfs_unitTest.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) {
        TEST_PRINTF("[ERROR] create file failed: %s\n", FR_Table[fr]);
        return -1;
    }

    TEST_PRINTF("[INFO] file create success: fatfs_unitTest.txt\n");

    fr = f_write(&file, test_data, strlen(test_data), &bw);
    if (fr != FR_OK || bw != strlen(test_data)) {
        TEST_PRINTF("[ERROR] file write failed: %s, bw=%u (expect: %zu)\n", 
                FR_Table[fr], bw, strlen(test_data));
        f_close(&file);
        return -2;
    }

    TEST_PRINTF("[INFO] write success: %u b\n", bw);
    TEST_PRINTF("[INFO] write content: %s\n", test_data);

    fr = f_close(&file);
    if (fr != FR_OK) {
        TEST_PRINTF("[ERROR] close file failed: %s \n", FR_Table[fr]);
        return -3;
    }

	return 0;
}

int test_read_and_verify(void)
{
    FRESULT fr;
    UINT br;
    const char *expected_data = "This is a UnitTest";

    fr = f_open(&file, "0:/fatfs_unitTest.txt", FA_READ);
    if (fr != FR_OK) {
        TEST_PRINTF("[ERROR] open file failed: %s\n", FR_Table[fr]);
        return -1;
    }

    TEST_PRINTF("[INFO] file open success\n");

    fr = f_read(&file, read_buffer, sizeof(read_buffer) - 1, &br);
    if (fr != FR_OK) {
        TEST_PRINTF("[ERROR] read file failed: %s \n", FR_Table[fr]);
        f_close(&file);
        return -2;
    }

    read_buffer[br] = '\0';

    TEST_PRINTF("[INFO] read success: %ub\n", br);
    TEST_PRINTF("[INFO] read content: %s\n", read_buffer);

    fr = f_close(&file);
    if (fr != FR_OK) {
        TEST_PRINTF("[ERROR] close file failed: %s\n", FR_Table[fr]);
        return -3;
    }

    if (br != strlen(expected_data) || 
        strcmp(read_buffer, expected_data) != 0) {
        TEST_PRINTF("[ERROR] content verify failed\n");
        TEST_PRINTF("  expect: %s (len: %zu)\n", expected_data, strlen(expected_data));
        TEST_PRINTF("  actual: %s (len: %u)\n", read_buffer, br);
        return -4;
    }

    TEST_PRINTF("[INFO] content verify success\n");
    return 0;
}

int test_file_info(void)
{
#if FF_USE_FIND
    FRESULT fr;
    FILINFO fno;

    fr = f_stat("0:/fatfs_unitTest.txt", &fno);
    if (fr != FR_OK) {
        TEST_PRINTF("[ERROR] get file stat failed : %s\n", FR_Table[fr]);
        return TEST_ERROR;
    }

    TEST_PRINTF("[INFO] file content:\n");
    TEST_PRINTF("  name: %s\n", fno.fname);
    TEST_PRINTF("  size: %lu b\n", fno.fsize);
    TEST_PRINTF("  create time: %u-%02u-%02u %02u:%02u\n",
            (fno.fdate >> 9) + 1980, (fno.fdate >> 5) & 15, fno.fdate & 31,
            fno.ftime >> 11, (fno.ftime >> 5) & 63);

    return 0;
#else
    TEST_PRINTF("[WARN] FF_USE_FIND not enable, jump file stat test\n");
    return 0;
#endif
}

int fatfs_test(int argc, char **argv)
{
    int ret = 0;

#if !defined(CONFIG_CRUSB_DEVICE_MSC_ENABLE)
    ff_ls("0:/");

	ff_create_and_write();

	test_read_and_verify();

	test_file_info();

    ff_ls("0:/");
#endif

	return ret;
}

#define TEST_FILE_COUNT  5
#define TEST_FILE_SIZE   (2 * 1024 * 1024)
#define BUFFER_SIZE      4096

#define CHECK_FRESULT(res, msg) do { \
    if ((res) != FR_OK) { \
        TEST_PRINTF("[ERROR] %s: %d (%s)\n", msg, res, FR_Table[res]); \
        return res; \
    } \
} while(0)

void print_progress(uint32_t *last_print, uint32_t now)
{
    if (now - *last_print >= 500) {
        TEST_PRINTF(".");
        *last_print = now;
    }
}

void print_size(uint64_t bytes)
{
    if (bytes >= 1024ULL * 1024 * 1024 * 1024) {
        TEST_PRINTF("%.2f TB", (double)bytes / (1024ULL * 1024 * 1024 * 1024));
    } else if (bytes >= 1024 * 1024 * 1024) {
        TEST_PRINTF("%.2f GB", (double)bytes / (1024 * 1024 * 1024));
    } else if (bytes >= 1024 * 1024) {
        TEST_PRINTF("%.2f MB", (double)bytes / (1024 * 1024));
    } else if (bytes >= 1024) {
        TEST_PRINTF("%.2f KB", (double)bytes / 1024);
    } else {
        TEST_PRINTF("%llu B", bytes);
    }
}

void print_capacity(const char *path)
{
    FATFS *pfs;
    DWORD free_clust;
    FRESULT res = f_getfree(path, &free_clust, &pfs);
    if (res != FR_OK) {
        TEST_PRINTF("Cannot get free space: %d\n", res);
        return;
    }
    uint64_t total_sectors = (pfs->n_fatent - 2) * pfs->csize;
    uint64_t free_sectors = free_clust * pfs->csize;
    uint64_t total_bytes = total_sectors * pfs->ssize;
    uint64_t free_bytes = free_sectors * pfs->ssize;

    TEST_PRINTF("Total space: ");
    print_size(total_bytes);
    TEST_PRINTF(", Free space: ");
    print_size(free_bytes);
    TEST_PRINTF("\n");
}

BYTE write_buf[BUFFER_SIZE];
BYTE read_buf[BUFFER_SIZE];
int fatfs_total_test(int argc, char **argv)
{
    int ret = 0;
    FRESULT res;
    FIL fil;
    DIR dir;
    FILINFO fno;
    char path[64];
    UINT bw, br;
    uint32_t start_tick, end_tick;
    uint32_t total_write_time = 0, total_read_time = 0;
    uint32_t total_written, total_read;
    int i;

#if !defined(CONFIG_CRUSB_DEVICE_MSC_ENABLE)
    ff_ls("0:/");

    print_capacity("0:");

    TCHAR cwd[64];
    ret = f_getcwd(cwd, sizeof(cwd));
    if (ret == FR_OK) {
        TEST_PRINTF("Current directory: %s\n", cwd);
    } else {
        TEST_PRINTF("f_getcwd error: %d\n", ret);
    }

    ret = f_mkdir("0:/test_case");
    if (ret != FR_OK && ret != FR_EXIST) {
        CHECK_FRESULT(ret, "f_mkdir");
    } else if (ret == FR_EXIST) {
        TEST_PRINTF("Directory already exists, will overwrite.\n");
    }

    ret = f_chdir("0:/test_case");
    CHECK_FRESULT(ret, "f_chdir");
    ret = f_getcwd(cwd, sizeof(cwd));
    if (ret == FR_OK) {
        TEST_PRINTF("Changed to: %s\n", cwd);
    }

    for (int j = 0; j < BUFFER_SIZE; j++) {
        write_buf[j] = (j & 0xFF);
    }

    for (i = 0; i < TEST_FILE_COUNT; i++) {
        TEST_PRINTF("\n--- Testing file %d/%d ---\n", i+1, TEST_FILE_COUNT);
        sprintf(path, "speed_test_%d.txt", i+1);

        res = f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE);
        CHECK_FRESULT(res, "f_open (write)");
        TEST_PRINTF("Writing %u bytes to %s ...\n", TEST_FILE_SIZE, path);
        start_tick = board_get_time();
        total_written = 0;
        uint32_t last_print = start_tick;

        while (total_written < TEST_FILE_SIZE) {
            UINT to_write = (TEST_FILE_SIZE - total_written) > BUFFER_SIZE ? BUFFER_SIZE : (TEST_FILE_SIZE - total_written);
            res = f_write(&fil, write_buf, to_write, &bw);
            if (res != FR_OK || bw != to_write) {
                TEST_PRINTF("Write error: res=%d, bw=%u, expected=%u\n", res, bw, to_write);
                f_close(&fil);
                return res ? res : FR_DISK_ERR;
            }
            total_written += bw;

            uint32_t now = board_get_time();
            print_progress(&last_print, now);
        }
        TEST_PRINTF("\n");
        end_tick = board_get_time();
        uint32_t write_time = end_tick - start_tick;
        total_write_time += write_time;
        double write_speed = (double)TEST_FILE_SIZE / write_time * 1000.0 / 1024.0;
        board_get_time("Write done in %lu ms, speed = %.2f KB/s\n", write_time, write_speed);

        res = f_close(&fil);
        CHECK_FRESULT(res, "f_close (write)");

        res = f_open(&fil, path, FA_READ);
        CHECK_FRESULT(res, "f_open (read)");
        printf("Reading and verifying %s ...\n", path);
        start_tick = board_get_time();
        total_read = 0;
        last_print = start_tick;
        int error = 0;

        while (total_read < TEST_FILE_SIZE) {
            UINT to_read = (TEST_FILE_SIZE - total_read) > BUFFER_SIZE ? BUFFER_SIZE : (TEST_FILE_SIZE - total_read);
            res = f_read(&fil, read_buf, to_read, &br);
            if (res != FR_OK || br != to_read) {
                TEST_PRINTF("Read error: res=%d, br=%u, expected=%u\n", res, br, to_read);
                f_close(&fil);
                return res ? res : FR_DISK_ERR;
            }
            for (UINT j = 0; j < br; j++) {
                if (read_buf[j] != write_buf[j % BUFFER_SIZE]) {
                    TEST_PRINTF("Data mismatch at offset %u (read=0x%02X, expected=0x%02X)\n",
                                total_read + j, read_buf[j], write_buf[j % BUFFER_SIZE]);
                    error = 1;
                    break;
                }
            }
            if (error) break;
            total_read += br;

            uint32_t now = board_get_time();
            print_progress(&last_print, now);
        }
        TEST_PRINTF("\n");
        end_tick = board_get_time();
        uint32_t read_time = end_tick - start_tick;
        total_read_time += read_time;
        double read_speed = (double)TEST_FILE_SIZE / read_time * 1000.0 / 1024.0;
        TEST_PRINTF("Read done in %lu ms, speed = %.2f KB/s\n", read_time, read_speed);

        if (error) {
            TEST_PRINTF("Verification failed!\n");
            f_close(&fil);
            return FR_INT_ERR;
        } else {
            TEST_PRINTF("Verification PASSED.\n");
        }

        res = f_close(&fil);
        CHECK_FRESULT(res, "f_close (read)");

        res = f_stat(path, &fno);
        if (res == FR_OK) {
            TEST_PRINTF("File size: %lu bytes, attributes: 0x%02X\n", (DWORD)fno.fsize, fno.fattrib);
        }
    }

    TEST_PRINTF("\n=== Average Write Speed: %.2f KB/s ===\n",
           (double)TEST_FILE_SIZE * TEST_FILE_COUNT / total_write_time * 1000.0 / 1024.0);
    TEST_PRINTF("=== Average Read Speed:  %.2f KB/s ===\n",
           (double)TEST_FILE_SIZE * TEST_FILE_COUNT / total_read_time * 1000.0 / 1024.0);

    TEST_PRINTF("\nDeleting test files...\n");
    res = f_opendir(&dir, ".");
    CHECK_FRESULT(res, "f_opendir");
    int deleted_count = 0;
    while (1) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) {
            break;
        }

        if (fno.fname[0] == '.' || (fno.fattrib & AM_DIR)) {
            continue;
        }

        if (strncmp(fno.fname, "speed_test_", 11) == 0) {
            res = f_unlink(fno.fname);
            if (res == FR_OK) {
                TEST_PRINTF("Deleted: %s\n", fno.fname);
                deleted_count++;
            } else {
                TEST_PRINTF("Failed to delete %s: %d\n", fno.fname, res);
            }
        }
    }

    f_closedir(&dir);
    TEST_PRINTF("Deleted %d files.\n", deleted_count);

    res = f_chdir("/");
    CHECK_FRESULT(res, "f_chdir root");
    res = f_rmdir("test_case");
    if (res == FR_OK) {
        TEST_PRINTF("Directory 'test_case' removed.\n");
    } else { 
        TEST_PRINTF("Failed to remove directory: %d\n", res);
    }

    TEST_PRINTF("--- After test ---\n");
    print_capacity("0:");

    TEST_PRINTF("\nTest suite completed.\n");

#endif
	return ret;
}
