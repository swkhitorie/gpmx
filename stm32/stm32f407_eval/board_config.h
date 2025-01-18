#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Simple STM32F407VET6 Board Config Header File
 * HSE 8MHz
 */
#include <stm32f4xx_hal.h>
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08000000)
#define HSE_VALUE             (8000000U)
#define LSE_VALUE             (32768UL)
#define __FPU_PRESENT         1
#define __FPU_USED            1

#define STM32_PLLCFG_PLL1M       (8)
#define STM32_PLLCFG_PLL1N       (336)
#define STM32_PLLCFG_PLL1P       (2)
#define STM32_PLLCFG_PLL1Q       (4)

#define STM32_PLL1P_FREQUENCY   \
                (((HSE_VALUE/STM32_PLLCFG_PLL1M)*STM32_PLLCFG_PLL1N)/STM32_PLLCFG_PLL1P)

#define STM32_SYSCLK_FREQUENCY  (STM32_PLL1P_FREQUENCY)
#define STM32_CPUCLK_FREQUENCY  (STM32_SYSCLK_FREQUENCY/1)
#define STM32_HCLK_FREQUENCY    (STM32_CPUCLK_FREQUENCY/2)
#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY/2)
#define STM32_PCLK2_FREQUENCY   (STM32_HCLK_FREQUENCY/2)

#ifdef __cplusplus
extern "C" {
#endif

void board_irq_reset();

void board_reboot();

/**
 * initialize board
 * 1. set vector for app
 * 2. reset all interrupt
 * 3. config hal systick, power, rcc clock
 * 4. config io
 * 5. call board_usb_init()
 */
void board_init();

/**
 * initialize bsp
 * 1. serial com, with redirectly printf
 * 2. ...
 */
void board_bsp_init();

void board_debug();

#ifdef __cplusplus
}
#endif










#endif
