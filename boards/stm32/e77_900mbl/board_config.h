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
#include <device/rtc.h>

#ifndef APP_LOAD_ADDRESS
#define APP_LOAD_ADDRESS      (0x08000000)
#endif

// #define LSE_VALUE             (32768UL)
// #define __FPU_USED            1
// #define __FPU_PRESENT         1    /* need change __FPU_PRESENT macro in stm32wlE5xx.h into 1UL */

#define USE_BSP_DRIVER

#define BOARD_PRINTF(...)    board_printf(__VA_ARGS__)

#define BOARD_CRUSH_PRINTF(...)    board_stream_printf(0, __VA_ARGS__)

#define BOARD_FIRMWARE_NAME   "ebyte_e77_900mbl"
#define BOARD_HARDWARE_VERSION   "v1.0.0"

#ifndef BOARD_SOFTWARE_VERSION
#define BOARD_SOFTWARE_VERSION   "v1.0.1"
#endif

#define BOARD_DEBUG(...) printf(__VA_ARGS__)

#define STM32_SYSCLK_FREQUENCY  (48000000)
#define STM32_CPUCLK_FREQUENCY  (STM32_SYSCLK_FREQUENCY/1)
#define STM32_HCLK_FREQUENCY    (STM32_CPUCLK_FREQUENCY/1)
#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY/1)
#define STM32_PCLK2_FREQUENCY   (STM32_HCLK_FREQUENCY/1)

#define STM32_APB1_TIM2_CLKIN   (1*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (1*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM4_CLKIN   (1*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM5_CLKIN   (1*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM6_CLKIN   (1*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM7_CLKIN   (1*STM32_PCLK1_FREQUENCY)

#define STM32_APB2_TIM1_CLKIN   (1*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM8_CLKIN   (1*STM32_PCLK2_FREQUENCY)

#ifdef __cplusplus
extern "C" {
#endif

void board_rng_init();
void board_rng_deinit();
void board_crc_init();
void board_crc_deinit();
void board_subghz_init();
void board_subghz_deinit();

void board_init();
void board_deinit();
void board_bsp_init();
void board_bsp_deinit();

void board_reboot();
void board_get_uid(uint32_t *p);
uint32_t board_get_time();
void     board_delay(uint32_t ms);
uint32_t board_elapsed_time(const uint32_t timestamp);
/*-------------- board bsp interface --------------*/

void board_test();
void board_led_toggle(uint8_t idx);  //0:red, 1:green, 2:blue

int  board_stream_in(int port, void *p, int size);
int  board_stream_out(int port, const void *p, int size, int way);
void board_stream_printf(int port, const char *format, ...);
void board_printf(const char *format, ...);

bool board_rtc_set_timestamp(rclk_time_t now);
rclk_time_t board_rtc_get_timestamp(struct rclk_timeval *now);

uint32_t board_rng_get();
bool     board_subghz_tx_ready();
uint32_t board_crc_key_get(uint32_t *uid, uint32_t key);

#ifdef __cplusplus
}
#endif


#endif
