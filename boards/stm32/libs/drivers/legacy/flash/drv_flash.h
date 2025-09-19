#ifndef DRV_FLASH_H_
#define DRV_FLASH_H_

#include "drv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t stm32_flash_readword(uint32_t addr);

int  stm32_flash_erase(uint32_t addr, size_t size);
int  stm32_flash_write(uint32_t addr, const uint8_t *buf, size_t size);
int  stm32_flash_read(uint32_t addr, uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
