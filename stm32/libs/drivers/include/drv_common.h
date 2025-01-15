#ifndef DRV_INCLUDE_H_
#define DRV_INCLUDE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined (DRV_BSP_G0)
#include "stm32g0xx.h"
#include "core_cm0.h"
#define CONFIG_UART_PERIPHAL_NUM (4)
#define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#elif defined (DRV_BSP_F1)
#include "stm32f1xx.h"
#include "core_cm3.h"
#define CONFIG_UART_PERIPHAL_NUM (4)
#define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#elif defined (DRV_BSP_F4)
#include "stm32f4xx.h"
#include "core_cm4.h"
#define CONFIG_UART_PERIPHAL_NUM (6)
#define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#elif defined (DRV_BSP_H7)
#include "stm32h7xx.h"
#include "core_cm7.h"
#define CONFIG_UART_PERIPHAL_NUM (8)
#define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#endif

#include "drv_buffer.h"

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

