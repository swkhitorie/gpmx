#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Simple STM32WL55_EVAL Board Config Header File
 */
#include <stm32wlxx_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <device/serial.h>

#define APP_LOAD_ADDRESS      (0x08000000)
#define LSE_VALUE             (32768UL)
#define __FPU_USED            1
#define __FPU_PRESENT         1    /* need change __FPU_PRESENT macro in stm32wl55xx.h into 1UL */

#define USE_BSP_DRIVER

/** 
 * std stream macro:
 * CONFIG_BOARD_COM_STDINOUT
 * 
 * os macro:
 * CONFIG_BOARD_FREERTOS_ENABLE
 * 
 */

// #define RADIO_TRANSMITTER  1
// #define RADIO_RECEIVER     2
// #define RADIO_ROLE         RADIO_TRANSMITTER

#ifdef __cplusplus
extern "C" {
#endif

extern char role_str[2][16];

void board_irq_reset();

void board_reboot();

void board_init();

void board_bsp_init();

/*-------------- board bsp interface --------------*/
void board_debug();

void board_led_toggle(int i);

int board_get_role();

uart_dev_t *board_get_stream_transout();

uart_dev_t *board_get_stream_transin();

#ifdef __cplusplus
}
#endif


#endif
