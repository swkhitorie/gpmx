#ifndef DRV_INCLUDE_H_
#define DRV_INCLUDE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "drv_buffer.h"

/** 
 * Macros: DRV_G0, DRV_F1, DRV_F4, DRV_H7
 */

#ifndef CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM
#define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#endif

#ifndef CONFIG_UART_PERIPHAL_NUM
#define CONFIG_UART_PERIPHAL_NUM (4)
#endif

/*
 * memory link areas for h7 ram d1
 */
#define RAM_D1      __attribute__((section(".RAM_D1")))

enum __drv_rwway {
    RWPOLL,
    RWIT,
    RWDMA
};





#endif

