#ifndef DRV_FLASH_H_
#define DRV_FLASH_H_

#include <stdint.h>
#include <stddef.h>
#include "n32g430.h"

#ifdef __cplusplus
extern "C" {
#endif

int  n32_flash_erase(uint32_t addr, size_t size);
int  n32_flash_write(uint32_t addr, const uint8_t *buf, size_t size);
int  n32_flash_read(uint32_t addr, uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
