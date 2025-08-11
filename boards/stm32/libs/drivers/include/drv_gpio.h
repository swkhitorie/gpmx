#ifndef DRV_GPIO_H_
#define DRV_GPIO_H_

#include <stdint.h>
#include <stdbool.h>
#include "drv_common.h"

/**
 * Pinset: uint32_t pinset:
 * [0:3] pin num 0~15
 * [31:4] pin port
 */
#define __STM32_PORT(port)  port##_BASE
#if !defined(SOC_SERIES_STM32MP1)

#define PIN_PORT(pin) ((uint8_t)(((pin) >> 4) & 0xFu))
#define PIN_NO(pin) ((uint8_t)((pin)&0xFu))
#define GET_PIN(PORTx,PIN) (uint32_t)((16 * ( ((uint32_t)__STM32_PORT(PORTx) - (uint32_t)GPIOA_BASE)/(0x0400UL) )) + PIN)
#define GET_PINHAL(PORTx,PIN) (uint32_t)((16 * ( ((uint32_t)(PORTx) - (uint32_t)GPIOA_BASE)/(0x0400UL) )) + PIN)

#define PIN_STPORT(pin) ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin) ((uint16_t)(1u << PIN_NO(pin)))
#endif

/**
 * Simple gpio use macro:
 */
#define LOW_IOGET(port, pin)  HAL_GPIO_ReadPin(port, 1<<pin)
#define LOW_IOSET(port, pin, val)  HAL_GPIO_WritePin(port, 1<<pin, val)
#define LOW_INITPIN(port, pin, mode, pull, speed) \
        do { \
            __HAL_RCC_##port##_CLK_ENABLE(); \
            GPIO_InitTypeDef obj_tmp; \
            obj_tmp.Pin = 1<<pin; \
            obj_tmp.Mode = mode; \
            obj_tmp.Pull = pull; \
            obj_tmp.Speed = speed; \
            HAL_GPIO_Init(port, &obj_tmp); \
        } while(0)

/**
 * Periph gpio init use macro:
 */
#if !defined (DRV_BSP_F1)
#define LOW_PERIPH_INITPIN(port, pin, mode, pull, speed, alternate) \
        stm32_gpio_setup(port, pin, mode, pull, speed, alternate, NULL, NULL, 0)
#else
#define LOW_PERIPH_INITPIN(port, pin, mode, pull, speed) \
        stm32_gpio_setup(port, pin, mode, pull, speed, 0, NULL, NULL, 0)
#endif

enum pullstate {
    IO_NOPULL = GPIO_NOPULL,
    IO_PULLUP = GPIO_PULLUP,
    IO_PULLDOWN = GPIO_PULLDOWN,
};

enum iospeed {
    IO_SPEEDLOW = GPIO_SPEED_FREQ_LOW,
    IO_SPEEDMID = GPIO_SPEED_FREQ_MEDIUM,
    IO_SPEEDHIGH = GPIO_SPEED_FREQ_HIGH,
#if !defined (DRV_BSP_F1)
    IO_SPEEDMAX = GPIO_SPEED_FREQ_VERY_HIGH,
#endif
};

enum iomode {
    IOMODE_INPUT = GPIO_MODE_INPUT,
    IOMODE_OUTPP = GPIO_MODE_OUTPUT_PP,
    IOMODE_OUTOD = GPIO_MODE_OUTPUT_OD,
    IOMODE_AFPP = GPIO_MODE_AF_PP,
    IOMODE_AFOD = GPIO_MODE_AF_OD,
    IOMODE_ANALOG = GPIO_MODE_ANALOG,
    IOMODE_IT_RISING = GPIO_MODE_IT_RISING,
    IOMODE_IT_FALLING = GPIO_MODE_IT_FALLING,
    IOMODE_IT_BOTH = GPIO_MODE_IT_RISING_FALLING,
};

typedef void (*io_exit_func)(void *arg);

struct gpio_pin_t {
    GPIO_TypeDef *port;   
    uint16_t pin;
    uint32_t alternate;

    io_exit_func callback;
    void *arg;
    uint32_t priority;
};

#ifdef cplusplus
extern "C" {
#endif

struct gpio_pin_t stm32_gpio_setup(
    GPIO_TypeDef *port, uint32_t pin, uint32_t mode, 
    uint32_t pull, uint32_t speed, uint32_t alternate, 
    io_exit_func entry, void *arg, uint32_t priority);

void stm32_gpio_write(struct gpio_pin_t *obj, uint8_t val);

uint8_t stm32_gpio_read(struct gpio_pin_t *obj);

/**
 * Pinset handle func
 */
bool stm32_gpioread(uint32_t pinset);

void stm32_gpiowrite(uint32_t pinset, bool value);

int stm32_gpiosetevent(uint32_t pinset, bool risingedge, bool fallingedge,
                       bool event, io_exit_func func, void *arg, uint32_t priority);

#ifdef cplusplus
}
#endif

#endif
