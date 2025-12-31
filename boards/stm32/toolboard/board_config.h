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

#define BOARD_PRINTF(...)    board_printf(__VA_ARGS__)

#define BOARD_CRUSH_PRINTF(...)    board_stream_printf(0, __VA_ARGS__)

#define BOARD_FIRMWARE_NAME   "toolboard"
#define BOARD_HARDWARE_VERSION   "v1.0.0"

#ifndef BOARD_SOFTWARE_VERSION
#define BOARD_SOFTWARE_VERSION   "v1.0.1"
#endif

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

void board_reboot();

void board_init();
void board_deinit();

void board_bsp_init();
void board_bsp_deinit();

/*-------------- board bsp interface --------------*/
void board_led_toggle(uint8_t idx);

void board_get_uid(uint32_t *p);

uint32_t board_get_time();
void     board_delay(uint32_t ms);
uint32_t board_elapsed_time(const uint32_t timestamp);

int  board_stream_in(int port, void *p, int size);
int  board_stream_out(int port, const void *p, int size, int way);
void board_stream_printf(int port, const char *format, ...);
void board_printf(const char *format, ...);

void board_debug();

#ifdef __cplusplus
}
#endif










#endif
