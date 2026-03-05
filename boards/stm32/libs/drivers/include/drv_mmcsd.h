#ifndef DRIVE_MMC_SD_H_
#define DRIVE_MMC_SD_H_

#include "drv_cmn.h"
#include "drv_gpio.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int  hw_stm32_mmcsd_init(int controller, uint32_t speed, uint8_t priority);

void hw_stm32_mmcsd_info(int controller);

#ifdef CONFIG_STM32_MMCSD_FATFS_ENABLE
void hw_stm32_mmcsd_fs_init(int controller);
#endif

#ifdef __cplusplus
}
#endif

#endif

