#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#include "gd32c10x.h"

#include "drv_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t board_tickget();

void board_bsp_init();

void board_init();

void board_debug();

#ifdef __cplusplus
}
#endif

#endif

