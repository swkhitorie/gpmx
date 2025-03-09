#ifndef DRV_SDMMC_H_
#define DRV_SDMMC_H_

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

struct drv_sdmmc_attr_t {
    uint8_t num;
    uint32_t speed;  //50Mhz, 100Mhz ....
    uint8_t d0s;
    uint8_t d1s;
    uint8_t d2s;
    uint8_t d3s;
    uint8_t cmds;
    uint8_t clks;
    uint8_t priority;
};

struct drv_sdmmc_t {
    bool initalized;
    int initret;
    SD_HandleTypeDef handle;
	HAL_SD_CardCIDTypedef cid;
	HAL_SD_CardCSDTypedef csd;
	HAL_SD_CardInfoTypeDef info;
    struct drv_sdmmc_attr_t attr;
};

extern struct drv_sdmmc_t *drv_sdmmc_list[2];

#ifdef __cplusplus
extern "C" {
#endif

void drv_sdmmc_attr_init(struct drv_sdmmc_attr_t *obj, uint8_t num, uint8_t speed,
                    uint8_t d0s, uint8_t d1s, uint8_t d2s, uint8_t d3s,
                    uint8_t cmds, uint8_t clks, uint8_t priority);
int drv_sdmmc_init(struct drv_sdmmc_t *obj, struct drv_sdmmc_attr_t *attr);
void drv_sdmmc_getinfo(struct drv_sdmmc_t *obj);
uint8_t drv_sdmmc_getstate(struct drv_sdmmc_t *obj);
uint8_t drv_sdmmc_wait_ready(struct drv_sdmmc_t *obj);

uint8_t drv_sdmmc_erase(struct drv_sdmmc_t *obj, uint32_t start, uint32_t end);
uint8_t drv_sdmmc_read_blocks(struct drv_sdmmc_t *obj, 
    uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way);
uint8_t drv_sdmmc_write_blocks(struct drv_sdmmc_t *obj, 
    uint32_t *p, uint32_t addr, uint32_t num, enum __drv_rwway way);

#ifdef __cplusplus
}
#endif

#endif
