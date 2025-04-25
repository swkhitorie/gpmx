#ifndef DRV_MMCSD_H_
#define DRV_MMCSD_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"
#if defined (DRV_BSP_H7)
#include "drv_pin_h7.h"
#endif

#define   SD_TRANSFER_OK                ((uint8_t)0x00)
#define   SD_TRANSFER_BUSY              ((uint8_t)0x01)

#define   MSD_OK                        ((uint8_t)0x00)
#define   MSD_ERROR                     ((uint8_t)0x01)
#define   MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)0x02)
#define   SD_TIMEOUT				    (5 * 1000)

struct up_mmcsd_dev_s
{
    uint32_t speed; //50Mhz, 100Mhz ....

    uint8_t id;
    uint8_t pin_cmd;
    uint8_t pin_d0;
    uint8_t pin_d1;
    uint8_t pin_d2;
    uint8_t pin_d3;
    uint8_t pin_clks;
    uint8_t priority;

    bool initalized;
    int initret;
    SD_HandleTypeDef handle;
	HAL_SD_CardCIDTypedef cid;
	HAL_SD_CardCSDTypedef csd;
	HAL_SD_CardInfoTypeDef info;
};

#ifdef __cplusplus
extern "C" {
#endif

int     low_mmcsd_init(struct up_mmcsd_dev_s *dev);

void    low_mmcsd_getinfo(struct up_mmcsd_dev_s *dev);
uint8_t low_mmcsd_getstat(struct up_mmcsd_dev_s *dev);
uint8_t low_mmcsd_waitrdy(struct up_mmcsd_dev_s *dev);

uint8_t low_mmcsd_erase(struct up_mmcsd_dev_s *dev, uint32_t start, uint32_t end);
uint8_t low_mmcsd_reads(struct up_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way);
uint8_t low_mmcsd_writes(struct up_mmcsd_dev_s *dev, uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way);

#ifdef __cplusplus
}
#endif

#endif
