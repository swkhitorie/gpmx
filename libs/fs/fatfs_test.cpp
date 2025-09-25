#include "fatfs_test.h"

#include <stdio.h>
#include <string.h>

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

void ff_ls(const char *path)
{
	FRESULT result;
    FILINFO fileinfo;
    DIR dirinfo;
	uint32_t cnt = 0;

    memset(&fileinfo, 0, sizeof(FILINFO));
    memset(&dirinfo, 0, sizeof(DIR));

	result = f_opendir(&dirinfo, path);
	if (result != FR_OK) {
		printf("[fat] open root dir failed %s \r\n", FR_Table[result]);
	}

	printf("properties  |  size |    name     \r\n");
	for (cnt = 0; ;cnt++) {
		result = f_readdir(&dirinfo, &fileinfo);
		if (result != FR_OK || fileinfo.fname[0] == 0) {
			break;
		}

		if (fileinfo.fname[0] == '.') {
			continue;
		}

		if (fileinfo.fattrib & AM_DIR) {
			printf("dir(0x%x)\t",(uint32_t)fileinfo.fattrib);
		}else {
			printf("file(0x%x)\t",(uint32_t)fileinfo.fattrib);
		}

		printf(" %d \t",(uint32_t)fileinfo.fsize);
		printf(" %s\r\n",fileinfo.fname);
	}
}

// ff.c line 2928 create_name() func:
// memset(dp->fn, ' ', 11) change to memset(dp->fn, ' ', 12) ??
uint8_t rd_buf[1024];
FIL rd_fp;
void ff_cat(char *path)
{
	FRESULT ret;
	UINT bw;

	ret = f_open(&rd_fp, path, FA_OPEN_EXISTING | FA_READ);
	if (ret != FR_OK) {
		printf("[fat] %s open file failed: %s \r\n", path, FR_Table[ret]);
		return;
	}

	ret = f_read(&rd_fp, rd_buf, rd_fp.obj.objsize, &bw);
	if (ret != FR_OK) {
		printf("[fat] %s read file failed %s \r\n", path, FR_Table[ret]);
        return;
	}

    printf("file: %s:\r\n", path);
    printf("%s", rd_buf);

	f_close(&rd_fp);
}
