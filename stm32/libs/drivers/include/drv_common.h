#ifndef DRV_INCLUDE_H_
#define DRV_INCLUDE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "drv_buffer.h"
#if defined (DRV_BSP_G0)
#include "stm32g0xx.h"
#include "core_cm0plus.h"
#elif defined (DRV_BSP_F1)
#include "stm32f1xx.h"
#include "core_cm3.h"
#elif defined (DRV_BSP_F4)
#include "stm32f4xx.h"
#include "core_cm4.h"
#elif defined (DRV_BSP_H7)
#include "stm32h7xx.h"
#include "core_cm7.h"
#endif

/**
 * level range: [1 ~ 3(MAX)]
 * Max Default
 */
#define BSP_CHIP_RESOURCE_MAX   (5)

#if defined (DRV_BSP_H7)
    #define BSP_CHIP_RESOURCE_LEVEL BSP_CHIP_RESOURCE_MAX
    #define CONFIG_UART_PERIPHAL_NUM (8)
    #define CONFIG_SPI_PERIPHAL_NUM  (6)
    #define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#elif defined (DRV_BSP_F1)
    #define BSP_CHIP_RESOURCE_LEVEL 1
    #define CONFIG_UART_PERIPHAL_NUM (3)
    #define CONFIG_SPI_PERIPHAL_NUM  (2)
    #define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#elif defined (DRV_BSP_F4)
    #if defined(STM32F407xx)
        #define BSP_CHIP_RESOURCE_LEVEL 2
    #else
        #define BSP_CHIP_RESOURCE_LEVEL BSP_CHIP_RESOURCE_MAX
    #endif
    #define CONFIG_UART_PERIPHAL_NUM (6)
    #define CONFIG_SPI_PERIPHAL_NUM  (6)
    #define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
#elif defined (DRV_BSP_G0)
    #define BSP_CHIP_RESOURCE_LEVEL 1
    #define CONFIG_UART_PERIPHAL_NUM (4)
    #define CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM (16)
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

