#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/**
 * PIXHAWK FMU V6C Board Config Header File (STM32H743VIT6) rely on STM32CubeH7 v1.11.2
 * HSE 16MHZ
 *  bug 1: v6c can not enter SysTick_Handler() irq
 *        -> fixed: call __set_BASEPRI(0); of set it priority higher, default 15
 *  bug 2: v6c can not enter OTG_FS_IRQHandler() irq
 *        change h7 usb's clock source HSI48 -> PLL3Q (48M)
 *        edit VID and PID to default 0xFFFF, why????
 *  bug 3: porting freertos v10.2.1
 *        enter hardfault in prvInitialiseNewTask() (called by xTaskCreate and static )
 *        change configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 -> 9????
 */
#include <stm32h7xx_hal.h>
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08020000)

#define HSE_VALUE             (16000000UL)
#define LSE_VALUE             (32768UL)
#define __FPU_PRESENT         1
#define __FPU_USED            1

#define GPIO_nLED_RED_PORT   (GPIOD)
#define GPIO_nLED_RED_PIN    (GPIO_PIN_10)

#define GPIO_nLED_BLUE_PORT   (GPIOD)
#define GPIO_nLED_BLUE_PIN    (GPIO_PIN_11)

#define GPIO_nPOWER_IN_A_PORT   (GPIOA)
#define GPIO_nPOWER_IN_A_PIN    (GPIO_PIN_15)

#define GPIO_nPOWER_IN_B_PORT   (GPIOB)
#define GPIO_nPOWER_IN_B_PIN    (GPIO_PIN_12)

#define GPIO_nPOWER_IN_C_PORT   (GPIOE)
#define GPIO_nPOWER_IN_C_PIN    (GPIO_PIN_15)

#define GPIO_VDD_5V_PERIPH_nEN_PORT  (GPIOE)
#define GPIO_VDD_5V_PERIPH_nEN_PIN  (GPIO_PIN_2)

#define GPIO_VDD_5V_PERIPH_nOC_PORT  (GPIOE)
#define GPIO_VDD_5V_PERIPH_nOC_PIN  (GPIO_PIN_3)

#define GPIO_VDD_5V_HIPOWER_nEN_PORT  (GPIOC)
#define GPIO_VDD_5V_HIPOWER_nEN_PIN  (GPIO_PIN_10)

#define GPIO_VDD_5V_HIPOWER_nOC_PORT  (GPIOC)
#define GPIO_VDD_5V_HIPOWER_nOC_PIN  (GPIO_PIN_11)

#define GPIO_VDD_3V3_SENSORS_EN_PORT  (GPIOB)
#define GPIO_VDD_3V3_SENSORS_EN_PIN  (GPIO_PIN_2)

#define GPIO_OTGFS_VBUS_PORT           (GPIOA)
#define GPIO_OTGFS_VBUS_PIN           (GPIO_PIN_9)

#define VDD_5V_PERIPH_EN(on_true)          HAL_GPIO_WritePin(GPIO_VDD_5V_PERIPH_nOC_PORT, \
                                                    GPIO_VDD_5V_PERIPH_nEN_PIN, !(on_true))
#define VDD_5V_HIPOWER_EN(on_true)         HAL_GPIO_WritePin(GPIO_VDD_5V_HIPOWER_nEN_PORT, \
                                                    GPIO_VDD_5V_HIPOWER_nEN_PIN, !(on_true))
#define VDD_3V3_SENSORS_EN(on_true)       HAL_GPIO_WritePin(GPIO_VDD_3V3_SENSORS_EN_PORT, \
                                                    GPIO_VDD_3V3_SENSORS_EN_PIN, (on_true))

#define BOARD_ADC_USB_CONNECTED           (HAL_GPIO_ReadPin(GPIO_OTGFS_VBUS_PORT, \
                                                    GPIO_OTGFS_VBUS_PIN))

#define BOARD_ADC_USB_VALID               (!HAL_GPIO_ReadPin(GPIO_nPOWER_IN_C_PORT, \
                                                    GPIO_nPOWER_IN_C_PIN))

#define BOARD_ADC_BRICK1_VALID  (!HAL_GPIO_ReadPin(GPIO_nPOWER_IN_A_PORT, GPIO_nPOWER_IN_A_PIN))
#define BOARD_ADC_BRICK2_VALID  (!HAL_GPIO_ReadPin(GPIO_nPOWER_IN_B_PORT, GPIO_nPOWER_IN_B_PIN))

#define BOARD_ADC_PERIPH_5V_OC  (!HAL_GPIO_ReadPin(GPIO_VDD_5V_PERIPH_nOC_PORT, \
                                                    GPIO_VDD_5V_PERIPH_nOC_PIN))
#define BOARD_ADC_HIPOWER_5V_OC (!HAL_GPIO_ReadPin(GPIO_VDD_5V_HIPOWER_nOC_PORT, \
                                                    GPIO_VDD_5V_HIPOWER_nOC_PIN))
#define BOARD_BLUE_LED(on_true)            HAL_GPIO_WritePin(GPIO_nLED_BLUE_PORT, \
                                                    GPIO_nLED_BLUE_PIN, !(on_true))
#define BOARD_RED_LED(on_true)            HAL_GPIO_WritePin(GPIO_nLED_RED_PORT, \
                                                    GPIO_nLED_RED_PIN, !(on_true))

#ifdef __cplusplus
    extern "C" {
#endif

void board_irqreset();

void board_reboot();

void board_blue_led_toggle();

void board_red_led_toggle();

/**
 * initialize cherryusb module
 * 1. usb clock
 * 2. cherry usb
 * 3. usb dm dp
 */
void board_usb_init();

/**
 * initialize board
 * 1. set vector for app
 * 2. reset all interrupt
 * 3. config mpu, cache, hal systick, power, rcc clock
 * 4. config io
 * 5. call board_usb_init()
 */
void board_init();

#ifdef __cplusplus
}
#endif

#endif
