#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/**
 * PIXHAWK FMU V6C Board Config Header File (STM32H743VIT6)
 * HSE 16MHZ
 * verified stm32 driver: usart/i2c/spi
 * bug 1: v6c can not enter SysTick_Handler() irq
 *       -> fixed: call __set_BASEPRI(0); of set it priority higher, default 15
 * warning 1: v6c can not enter OTG_FS_IRQHandler() irq
 *       change h7 usb's clock source HSI48 -> PLL3Q (48M)
 *       edit VID and PID to default 0xFFFF, why????
 * bug 2: porting freertos v10.2.1
 *       enter hardfault in prvInitialiseNewTask() (called by xTaskCreate and static )
 *       change configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 -> 9????
 */
#include <stm32h7xx_hal.h>
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08020000)
#define HSE_VALUE             (16000000UL)
#define LSE_VALUE             (32768UL)
#define __FPU_PRESENT         1
#define __FPU_USED            1
#define __MPU_PRESENT         1

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

#define STM32_PLLCFG_PLL1M       (1)
#define STM32_PLLCFG_PLL1N       (60)
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

#define GPIO_nLED_RED_PORT   (GPIOD)
#define GPIO_nLED_RED_PIN    (10)

#define GPIO_nLED_BLUE_PORT   (GPIOD)
#define GPIO_nLED_BLUE_PIN    (11)

#define GPIO_nPOWER_IN_A_PORT   (GPIOA)
#define GPIO_nPOWER_IN_A_PIN    (15)

#define GPIO_nPOWER_IN_B_PORT   (GPIOB)
#define GPIO_nPOWER_IN_B_PIN    (12)

#define GPIO_nPOWER_IN_C_PORT   (GPIOE)
#define GPIO_nPOWER_IN_C_PIN    (15)

#define GPIO_VDD_5V_PERIPH_nEN_PORT  (GPIOE)
#define GPIO_VDD_5V_PERIPH_nEN_PIN  (2)

#define GPIO_VDD_5V_PERIPH_nOC_PORT  (GPIOE)
#define GPIO_VDD_5V_PERIPH_nOC_PIN  (3)

#define GPIO_VDD_5V_HIPOWER_nEN_PORT  (GPIOC)
#define GPIO_VDD_5V_HIPOWER_nEN_PIN  (10)

#define GPIO_VDD_5V_HIPOWER_nOC_PORT  (GPIOC)
#define GPIO_VDD_5V_HIPOWER_nOC_PIN  (11)

#define GPIO_VDD_3V3_SENSORS_EN_PORT  (GPIOB)
#define GPIO_VDD_3V3_SENSORS_EN_PIN  (2)

#define GPIO_OTGFS_VBUS_PORT           (GPIOA)
#define GPIO_OTGFS_VBUS_PIN           (9)

#define VDD_5V_PERIPH_EN(on_true)     BOARD_IO_SET(GPIO_VDD_5V_PERIPH_nEN_PORT,\
                                           GPIO_VDD_5V_PERIPH_nEN_PIN, !(on_true))
#define VDD_5V_HIPOWER_EN(on_true)     BOARD_IO_SET(GPIO_VDD_5V_HIPOWER_nEN_PORT,\
                                           GPIO_VDD_5V_HIPOWER_nEN_PIN, !(on_true))
#define VDD_3V3_SENSORS_EN(on_true)    BOARD_IO_SET(GPIO_VDD_3V3_SENSORS_EN_PORT,\
                                           GPIO_VDD_3V3_SENSORS_EN_PIN, (on_true))

#define BOARD_ADC_USB_CONNECTED           (BOARD_IO_GET(GPIO_OTGFS_VBUS_PORT, \
                                                    GPIO_OTGFS_VBUS_PIN))

#define BOARD_ADC_USB_VALID               (!BOARD_IO_GET(GPIO_nPOWER_IN_C_PORT, \
                                                    GPIO_nPOWER_IN_C_PIN))

#define BOARD_ADC_BRICK1_VALID  (!BOARD_IO_GET(GPIO_nPOWER_IN_A_PORT, GPIO_nPOWER_IN_A_PIN))
#define BOARD_ADC_BRICK2_VALID  (!BOARD_IO_GET(GPIO_nPOWER_IN_B_PORT, GPIO_nPOWER_IN_B_PIN))

#define BOARD_ADC_PERIPH_5V_OC  (!BOARD_IO_GET(GPIO_VDD_5V_PERIPH_nOC_PORT, \
                                                    GPIO_VDD_5V_PERIPH_nOC_PIN))
#define BOARD_ADC_HIPOWER_5V_OC (!BOARD_IO_GET(GPIO_VDD_5V_HIPOWER_nOC_PORT, \
                                                    GPIO_VDD_5V_HIPOWER_nOC_PIN))
#define BOARD_BLUE_LED(on_true)            BOARD_IO_SET(GPIO_nLED_BLUE_PORT, \
                                                    GPIO_nLED_BLUE_PIN, !(on_true))
#define BOARD_RED_LED(on_true)            BOARD_IO_SET(GPIO_nLED_RED_PORT, \
                                                    GPIO_nLED_RED_PIN, !(on_true))

#ifdef __cplusplus
    extern "C" {
#endif

void board_irq_reset();

void board_reboot();

void board_init();

void board_bsp_init();

/*-------------- board bsp interface --------------*/
void board_debug();

void board_blue_led_toggle();

void board_red_led_toggle();

#ifdef __cplusplus
}
#endif

#endif
