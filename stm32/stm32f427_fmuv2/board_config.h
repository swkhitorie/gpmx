#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Pixhawk FMU v2 Board Config Header File
 * HSE 24MHZ
 * verified stm32 driver: 
 */
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08004000)
#define HSE_VALUE             (24000000UL)
#define LSE_VALUE             (32768UL)
#define __FPU_PRESENT         1
#define __FPU_USED            1

#define STM32_PLLCFG_PLL1M       (24)
#define STM32_PLLCFG_PLL1N       (336)
#define STM32_PLLCFG_PLL1P       (2)
#define STM32_PLLCFG_PLL1Q       (7)

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

#define GPIO_nLED_PORT   (GPIOE)
#define GPIO_nLED_PIN    (GPIO_PIN_12)

#define BOARD_LED(on_true)   HAL_GPIO_WritePin(GPIO_nLED_PORT, \
                            GPIO_nLED_PIN, !(on_true))

#ifdef __cplusplus
    extern "C" {
#endif

void board_irqreset();

void board_reboot();

void board_init();

void board_bsp_init();

/*-------------- board bsp interface --------------*/
void board_led_toggle();

void board_debug();

#ifdef __cplusplus
}
#endif

#endif
