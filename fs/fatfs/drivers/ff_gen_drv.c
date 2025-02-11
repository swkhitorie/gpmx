#include "ff_gen_drv.h"

diskio_drv_t disk = {{0},{0},{0},0};

/**
  * @brief  Links a compatible diskio driver/lun id and increments the number of active
  *         linked drivers.
  * @note   The number of linked drivers (volumes) is up to 10 due to FatFs limits.
  * @param  drv: pointer to the disk IO Driver structure
  * @param  path: pointer to the logical drive path
  * @param  lun : only used for USB Key Disk to add multi-lun management
            else the parameter must be equal to 0
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t fatfs_link_drvex(const diskio_drv_t *drv, char *path, uint8_t lun)
{
  uint8_t ret = 1;
  uint8_t num = 0;

  if(disk.nbr < _VOLUMES) {
    disk.is_initialized[disk.nbr] = 0;
    disk.drv[disk.nbr] = drv;
    disk.lun[disk.nbr] = lun;
    num = disk.nbr++;
    path[0] = num + '0';
    path[1] = ':';
    path[2] = '/';
    path[3] = 0;
    ret = 0;
  }
  return ret;
}

uint8_t fatfs_link_drv(const Diskio_drvTypeDef *drv, char *path)
{
  return fatfs_link_drvex(drv, path, 0);
}

/**
  * @brief  Unlinks a diskio driver and decrements the number of active linked
  *         drivers.
  * @param  path: pointer to the logical drive path
  * @param  lun : not used
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t fatfs_unlink_drvex(char *path, uint8_t lun)
{
  uint8_t num = 0;
  uint8_t ret = 1;

  if(disk.nbr >= 1) {
    num = path[0] - '0';
    if(disk.drv[num] != 0) {
      disk.drv[num] = 0;
      disk.lun[num] = 0;
      disk.nbr--;
      ret = 0;
    }
  }
  return ret;
}

uint8_t fatfs_unlink_drv(char *path)
{
  return fatfs_unlink_drvex(path, 0);
}

uint8_t fatfs_get_drv_nbr(void)
{
  return disk.nbr;
}
