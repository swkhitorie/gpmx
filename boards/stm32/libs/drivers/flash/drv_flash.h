#ifndef DRIVE_FLASH_H_
#define DRIVE_FLASH_H_

#include "drv_cmn.h"

#define FLASH_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

#ifdef __cplusplus
extern "C" {
#endif

int  stm32_flash_erase(uint32_t addr, size_t size);
int  stm32_flash_write(uint32_t addr, const uint8_t *buf, size_t size);
int  stm32_flash_read(uint32_t addr, uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
