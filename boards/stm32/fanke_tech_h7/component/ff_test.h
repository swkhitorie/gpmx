#ifndef FATFS_TEST_BOARD_H_
#define FATFS_TEST_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

void ff_ls(const char *path);
void ff_cat(char *path);

#ifdef __cplusplus
}
#endif

#endif
