#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Simple STM32WLE5_EVAL Board Config Header File
 */
#include <stm32wlxx_hal.h>
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08000000)
#define LSE_VALUE             (32768UL)
#define __FPU_USED            1
#define __FPU_PRESENT         1    /* need change __FPU_PRESENT macro in stm32wlE5xx.h into 1UL */

/** 
 * std stream macro:
 * CONFIG_BOARD_COM_STDINOUT
 * 
 * os macro:
 * CONFIG_BOARD_FREERTOS_ENABLE
 * 
 */

#define RADIO_TRANSMITTER  1
#define RADIO_RECEIVER     2
#define RADIO_ROLE         RADIO_RECEIVER

#ifdef __cplusplus
extern "C" {
#endif

void board_irq_reset();

void board_reboot();

void board_init();

void board_bsp_init();

/*-------------- board bsp interface --------------*/
void board_debug();

void board_led_toggle(int i);

bool board_radio_sync();

int board_radio_rxbuf_write(uint8_t *p, uint16_t size);

void board_radio_txflag_clr();

bool board_radio_sync_already();

#ifdef __cplusplus
}
#endif


#endif
