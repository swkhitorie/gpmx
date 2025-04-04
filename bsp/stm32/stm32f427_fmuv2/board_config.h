#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Pixhawk FMU v2 Board Config Header File
 * HSE 24MHZ
 * verified stm32 driver: uart
 */
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08004000)
#define HSE_VALUE             (24000000UL)
#define LSE_VALUE             (32768UL)
#define __FPU_PRESENT         1
#define __FPU_USED            1

/** 
 * std stream macro:
 * CONFIG_BOARD_COM_STDINOUT
 * 
 * os macro:
 * CONFIG_BOARD_FREERTOS_ENABLE
 * 
 * usb macro:
 * CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
 * 
 */

#define STM32_PLLCFG_PLL1M       (24)
#define STM32_PLLCFG_PLL1N       (336)
#define STM32_PLLCFG_PLL1P       (2)
#define STM32_PLLCFG_PLL1Q       (7)

#define STM32_PLL1P_FREQUENCY   \
                (((HSE_VALUE/STM32_PLLCFG_PLL1M)*STM32_PLLCFG_PLL1N)/STM32_PLLCFG_PLL1P)

#define STM32_SYSCLK_FREQUENCY  (STM32_PLL1P_FREQUENCY)
#define STM32_CPUCLK_FREQUENCY  (STM32_SYSCLK_FREQUENCY/1)
#define STM32_HCLK_FREQUENCY    (STM32_CPUCLK_FREQUENCY/1)

#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY/4)
#define STM32_PCLK2_FREQUENCY   (STM32_HCLK_FREQUENCY/2)

#define STM32_APB1_TIM2_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM4_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM5_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM6_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM7_CLKIN   (2*STM32_PCLK1_FREQUENCY)

#define STM32_APB2_TIM1_CLKIN   (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM8_CLKIN   (2*STM32_PCLK2_FREQUENCY)

#define BOARD_IO_GET(port, pin)  HAL_GPIO_ReadPin(port, (uint16_t)(0x01<<pin))
#define BOARD_IO_SET(port, pin, val)  HAL_GPIO_WritePin(port, (uint16_t)(0x01<<pin), val)
/* macro BOARD_INIT_IOPORT need user enable gpio clk */
#define BOARD_INIT_IOPORT(_num, port, pin, mode, pull, speed) \
        { \
            GPIO_InitTypeDef obj##_num; \
            obj##_num.Pin = (uint16_t)(0x01<<pin); \
            obj##_num.Mode = mode; \
            obj##_num.Pull = pull; \
            obj##_num.Speed = speed; \
            HAL_GPIO_Init(port, &obj##_num); \
        }

#define GPIO_nLED_PORT   (GPIOE)
#define GPIO_nLED_PIN    (12)

#define GPIO_VDD_5V_PERIPH_nEN_PORT  (GPIOA)
#define GPIO_VDD_5V_PERIPH_nEN_PIN   (8)

#define GPIO_VDD_3V3_SENSORS_nEN_PORT  (GPIOE)
#define GPIO_VDD_3V3_SENSORS_nEN_PIN   (3)

#define GPIO_VDD_5V_SENS_OC_PORT  (GPIOA)
#define GPIO_VDD_5V_SENS_OC_PIN   (4)

#define GPIO_VDD_5V_HIPOWER_OC_PORT  (GPIOE)
#define GPIO_VDD_5V_HIPOWER_OC_PIN   (10)

#define GPIO_VDD_5V_PERIPH_OC_PORT  (GPIOE)
#define GPIO_VDD_5V_PERIPH_OC_PIN   (15)

#define BOARD_LED(on_true)   BOARD_IO_SET(GPIO_nLED_PORT,GPIO_nLED_PIN,!(on_true))

#define VDD_5V_PERIPH_EN(on_true)  BOARD_IO_SET(GPIO_VDD_5V_PERIPH_nEN_PORT, \
                                    GPIO_VDD_5V_PERIPH_nEN_PIN, !(on_true))

#define VDD_3V3_SENSOR_EN(on_true)  BOARD_IO_SET(GPIO_VDD_3V3_SENSORS_nEN_PORT, \
                                    GPIO_VDD_3V3_SENSORS_nEN_PIN, (on_true))

#define VDD_5V_SENS_OC_VALID       (!BOARD_IO_GET(GPIO_VDD_5V_SENS_OC_PORT, \
                                    GPIO_VDD_5V_SENS_OC_PIN))

#define BOARD_ADC_PERIPH_5V_OC  (!BOARD_IO_GET(GPIO_VDD_5V_HIPOWER_OC_PORT, GPIO_VDD_5V_HIPOWER_OC_PIN))
#define BOARD_ADC_HIPOWER_5V_OC (!BOARD_IO_GET(GPIO_VDD_5V_PERIPH_OC_PORT, GPIO_VDD_5V_PERIPH_OC_PIN))

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
