#include "app_fatfs_debug.h"

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
		printf("[fat] open root dir failed %d \r\n", result);
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
void ff_display(char *path)
{
	FRESULT ret;
	UINT bw;

	ret = f_open(&rd_fp, path, FA_OPEN_EXISTING | FA_READ);
	if (ret != FR_OK) {
		printf("[fat] %s open file failed: %d \r\n", path, ret);
		return;
	}

	ret = f_read(&rd_fp, rd_buf, rd_fp.obj.objsize, &bw);
	if (ret != FR_OK) {
		printf("[fat] %s read file failed %s \r\n", path, ret);
        return;
	}

    printf("file: %s:\r\n", path);
    printf("%s", rd_buf);

	f_close(&rd_fp);
}
