#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#include "n32g430.h"
#include <stdint.h>
#include <stdbool.h>

// auto defined
// #define __FPU_PRESENT
// #define __FPU_USED

#ifdef __cplusplus
extern "C" {
#endif

uint32_t board_get_tmstamp(void);

void board_ms_delay(uint32_t ms);

uint32_t board_elapsed_tick(const uint32_t tick);

void board_reboot();

void board_init();

void board_get_uid(uint32_t *p);

void board_led_toggle(int idx);

void board_bsp_init();

uint8_t board_spi_exchange_byte(uint8_t val);

uint16_t board_tty_module_read(uint8_t *p, uint16_t len);

uint16_t board_tty_module_send(uint8_t *p, uint16_t len);

uint32_t board_rng_get();

#ifdef __cplusplus
}
#endif


#endif
