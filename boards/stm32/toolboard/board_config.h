#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Simple Evaluation Board Config Header File
 * cpu: stm32f103c8t6, oscillator: 8MHz
 *
 */
#include <stm32f1xx_hal.h>
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08000000)
// #define HSE_VALUE             (8000000U)
// #define LSE_VALUE             (32768UL)

#define STM32_PLLCFG_PLLMUL       (9)        /* RCC_PLL_MUL9 */

/** 
 * std stream macro:
 * CONFIG_BOARD_COM_STDINOUT
 * 
 * os macro:
 * CONFIG_BOARD_FREERTOS_ENABLE
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

void board_irq_reset();

void board_reboot();

void board_init();

void board_bsp_init();

/*-------------- board bsp interface --------------*/
void board_debug();

#ifdef __cplusplus
}
#endif










#endif
