#ifndef LITTLEFS_SPIFLASH_DRIVE_H_
#define LITTLEFS_SPIFLASH_DRIVE_H_

#include <lfs.h>

#ifdef __cplusplus
extern "C" {
#endif

int lfs_sflash_init(int nbus);
int lfs_sflasg_rwtest();

#ifdef __cplusplus
}
#endif


#endif
