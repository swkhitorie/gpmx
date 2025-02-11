#ifndef FF_GEN_DRV_H
#define FF_GEN_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "diskio.h"
#include "ff.h"
#include "stdint.h"

typedef struct {
  DSTATUS (*disk_initialize) (BYTE);
  DSTATUS (*disk_status)     (BYTE);
  DRESULT (*disk_read)       (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
  DRESULT (*disk_write)      (BYTE, const BYTE*, DWORD, UINT);
#endif
#if _USE_IOCTL == 1
  DRESULT (*disk_ioctl)      (BYTE, BYTE, void*);
#endif
} diskio_drv_ops_t;

typedef struct {
  uint8_t                 is_initialized[_VOLUMES];
  const diskio_drv_ops_t *drv[_VOLUMES];
  uint8_t                 lun[_VOLUMES];
  volatile uint8_t        nbr;
} diskio_drv_t;

uint8_t fatfs_link_drv(const diskio_drv_t *drv, char *path);
uint8_t fatfs_link_drvex(const diskio_drv_t *drv, char *path, BYTE lun);
uint8_t fatfs_unlink_drv(char *path);
uint8_t fatfs_unlink_drvex(char *path, BYTE lun);
uint8_t fatfs_get_drv_nbr(void);

#ifdef __cplusplus
}
#endif

#endif
