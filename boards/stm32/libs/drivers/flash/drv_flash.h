#ifndef DRV_FLASH_H_
#define DRV_FLASH_H_

#include "drv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void stm32_flash_read(uint32_t addr, uint32_t *p, uint32_t len);
int  stm32_flash_write(uint32_t addr, const uint32_t *p, uint32_t len);
int  stm32_flash_erase(uint32_t addr, size_t size);

#ifdef __cplusplus
}
#endif

#endif
