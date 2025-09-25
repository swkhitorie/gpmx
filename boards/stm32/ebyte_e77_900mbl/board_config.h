#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * EByte E77-900MBL-01 Board Config Header File
 * cpu: stm32wle5ccu8, oscillator: 32MHz
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
// #define __FPU_PRESENT         1    /* need change __FPU_PRESENT macro in stm32wlE5xx.h into 1UL */

#define USE_BSP_DRIVER
/** 
 * CONFIG_BOARD_COM_STDINOUT
 * CONFIG_BOARD_FREERTOS_ENABLE
 * 
 */

#define RADIO_BOARD_TRANSMITTER  1
#define RADIO_BOARD_RECEIVER     2
#ifndef RADIO_BOARD_ROLE
#define RADIO_BOARD_ROLE         RADIO_BOARD_TRANSMITTER
#endif

#define BOARD_DEBUG(...) do {\
    printf("[%d.%03d] ", HAL_GetTick()/1000, HAL_GetTick()%1000); \
    printf(__VA_ARGS__); \
    } while(0)
#ifdef __cplusplus
extern "C" {
#endif

extern uart_dev_t *_tty_log_out;
extern uart_dev_t *_tty_log_in;

extern uart_dev_t *_tty_msg_out;
extern uart_dev_t *_tty_msg_in;
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

int board_get_role();
#ifdef __cplusplus
}
#endif


#endif
