#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * FankeTech H7 Board Config Header File
 * cpu: stm32h743iit6, oscillator: 25HMz
 */
#include <stm32h7xx_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <drv_rtc.h>

#ifndef APP_LOAD_ADDRESS
#define APP_LOAD_ADDRESS      (0x08000000)
#endif

// #define HSE_VALUE             (25000000UL)
// #define LSE_VALUE             (32768UL)
// #define __FPU_PRESENT         1
// #define __FPU_USED            1

/** 
 * std stream macro:
 * CONFIG_BOARD_COM_STDINOUT
 * os macro:
 * CONFIG_BOARD_FREERTOS_ENABLE
 * usb macro:
 * CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
 * 
 */

#define STM32_PLLCFG_PLL1M       (5)
#define STM32_PLLCFG_PLL1N       (192) //160
#define STM32_PLLCFG_PLL1P       (2)
#define STM32_PLLCFG_PLL1Q       (4)

#define STM32_PLL1P_FREQUENCY   \
                (((HSE_VALUE/STM32_PLLCFG_PLL1M)*STM32_PLLCFG_PLL1N)/STM32_PLLCFG_PLL1P)

#define STM32_SYSCLK_FREQUENCY  (STM32_PLL1P_FREQUENCY)
#define STM32_CPUCLK_FREQUENCY  (STM32_SYSCLK_FREQUENCY/1)
#define STM32_HCLK_FREQUENCY    (STM32_CPUCLK_FREQUENCY/2)
#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY/2)
#define STM32_PCLK2_FREQUENCY   (STM32_HCLK_FREQUENCY/2)

#define STM32_APB1_TIM2_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM4_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM5_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM6_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM7_CLKIN   (2*STM32_PCLK1_FREQUENCY)

#define STM32_APB2_TIM1_CLKIN   (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM8_CLKIN   (2*STM32_PCLK2_FREQUENCY)

#ifdef __cplusplus
extern "C" {
#endif

void board_irq_reset();

void board_reboot();

void board_init();

void board_bsp_init();
void board_bsp_deinit();

/*-------------- board bsp interface --------------*/

void board_led_toggle(uint8_t idx);

void board_debug();

rclk_time_t board_rtc_get_timestamp(struct rclk_timeval *now);
bool   board_rtc_set_timestamp(rclk_time_t now);

#ifdef __cplusplus
}
#endif

#endif
