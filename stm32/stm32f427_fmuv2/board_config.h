#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/** 
 * Fanke STM32H427VET6 Evaluation Board Config Header File
 * HSE 24MHZ
 */
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define APP_LOAD_ADDRESS      (0x08004000)
#define HSE_VALUE             (24000000UL)
#define LSE_VALUE             (32768UL)
#define __FPU_PRESENT         1
#define __FPU_USED            1

#define GPIO_nLED_PORT   (GPIOE)
#define GPIO_nLED_PIN    (GPIO_PIN_12)

#define BOARD_LED(on_true)   HAL_GPIO_WritePin(GPIO_nLED_PORT, \
                            GPIO_nLED_PIN, !(on_true))

#ifdef __cplusplus
    extern "C" {
#endif

void board_irqreset();

void board_reboot();

void board_led_toggle();

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
 * 3. config hal systick, power, rcc clock
 * 4. config io
 * 5. call board_usb_init()
 */
void board_init();

#ifdef __cplusplus
}
#endif

#endif
