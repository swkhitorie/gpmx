#ifndef LFS_DRV_H
#define LFS_DRV_H

#include "lfs.h"

#define LFS_VOLUMES (5)

#ifdef __cplusplus
extern "C" {
#endif

void lfs_objset(lfs_t *obj, uint8_t num);
lfs_t *lfs_objget(uint8_t num);

#ifdef __cplusplus
}
#endif

#endif
