#include "lfs_drv.h"

lfs_t *lfs_array[LFS_VOLUMES];

void lfs_objset(lfs_t *obj, uint8_t num)
{
    lfs_array[num] = obj;
}

lfs_t *lfs_objget(uint8_t num)
{
    return lfs_array[num];
}
