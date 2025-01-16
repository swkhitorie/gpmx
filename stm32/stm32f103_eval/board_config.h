#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Simple STM32F103C8T6 Evaluation Board Config Header File
 * HSE 8MHz
 */
#include <stm32f1xx_hal.h>
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08000000)
#define HSE_VALUE             (8000000U)
#define LSE_VALUE             (32768UL)

#define STM32_PLLCFG_PLLMUL       (9)        /* RCC_PLL_MUL9 */

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
