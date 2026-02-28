#include "ff.h"
#include "fatfs_test.h"
#include <stdio.h>
#include <string.h>

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
        TEST_PRINTF("[ERROR] file write failed: %s, bw=%u (预期: %zu)\n", 
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

	ff_ls("0:/");

	ff_create_and_write();

	test_read_and_verify();

	test_file_info();

	return ret;
}
