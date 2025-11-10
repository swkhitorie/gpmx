#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Nucleo-WL55JC Board Config Header File
 * cpu: stm32wl55jclx core-m4, oscillator: 32MHz
 */
#include <stm32wlxx_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <device/serial.h>

#ifndef APP_LOAD_ADDRESS
#define APP_LOAD_ADDRESS      (0x08000000)
#endif

// #define LSE_VALUE             (32768UL)
// #define __FPU_USED            1
// #define __FPU_PRESENT         1    /* need change __FPU_PRESENT macro in stm32wl55xx.h into 1UL */

#define USE_BSP_DRIVER

/** 
 * CONFIG_BOARD_COM_STDINOUT
 * CONFIG_BOARD_FREERTOS_ENABLE
 */

#define BOARD_DEBUG(...) printf(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void board_irq_reset();

void board_reboot();

void board_init();
void board_deinit();

void board_bsp_init();
void board_bsp_deinit();

void board_rng_init();
void board_rng_deinit();

void board_crc_init();
void board_crc_deinit();

void board_subghz_init();
void board_subghz_deinit();

/*-------------- board bsp interface --------------*/

void board_debug();

uint32_t board_rng_get();

void board_get_uid(uint32_t *p);

//0:red, 1:green, 2:blue
void board_led_toggle(int i);

uint32_t board_elapsed_tick(const uint32_t tick);

bool board_subghz_tx_ready();

uint32_t board_crc_key_get(uint32_t *uid, uint32_t key);

#ifdef __cplusplus
}
#endif


#endif
