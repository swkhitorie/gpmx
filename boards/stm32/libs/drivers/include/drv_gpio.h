#ifndef DRIVE_GPIO_H_
#define DRIVE_GPIO_H_

#include <stdint.h>
#include <stdbool.h>
#include "drv_cmn.h"

/**
 * Pinset: uint32_t pinset:
 * [0:3] pin num 0~15
 * [31:4] pin port
 */

#define PIN_PORT(pin)        ((uint8_t)(((pin)>>4)&0xFu))
#define PIN_NO(pin)          ((uint8_t)((pin)&0xFu))
#define GET_PIN(PORTx,PIN)   (uint32_t)((16*(((uint32_t)(PORTx)-(uint32_t)GPIOA_BASE)/(0x0400UL)))+PIN)

#define PIN_STPORT(pin)      ((GPIO_TypeDef *)(GPIOA_BASE+(0x400u*PIN_PORT(pin))))
#define PIN_STPIN(pin)       ((uint16_t)(1u<<PIN_NO(pin)))


/**
 * Simple gpio use macro:
 */
#define LOW_IOGET(port, pin)       HAL_GPIO_ReadPin(port, 1<<pin)
#define LOW_IOSET(port, pin, val)  HAL_GPIO_WritePin(port, 1<<pin, (GPIO_PinState)val)
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
#if defined (DRV_STM32_F4) || defined(DRV_STM32_H7) || defined(DRV_STM32_WL)
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
#if defined (DRV_STM32_F4) || defined(DRV_STM32_H7) || defined(DRV_STM32_WL)
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

typedef void (*io_irq_entry)(void *arg);

struct io_pin_t {
    GPIO_TypeDef *port;
    uint16_t pin;
};

struct periph_pin_t {
    GPIO_TypeDef *port;
    uint16_t pin;
#if defined (DRV_STM32_F4) || defined(DRV_STM32_H7) || defined(DRV_STM32_WL)
    uint32_t alternate;
#endif
};

struct irq_pin_t {
    io_irq_entry callback;
    void *arg;
    uint32_t priority;
};

#ifdef cplusplus
extern "C" {
#endif

void    low_init_pin(GPIO_TypeDef* port, uint16_t pin, uint32_t mode, uint32_t pull, uint32_t speed);

void    stm32_gpio_setup(GPIO_TypeDef *port, uint32_t pin, 
            uint32_t mode, uint32_t pull, uint32_t speed, uint32_t alternate, 
            io_irq_entry entry, void *arg, uint32_t priority);

uint8_t stm32_pin_read(uint32_t pinset);
void    stm32_pin_write(uint32_t pinset, uint8_t value);
int     stm32_pin_setevent(uint32_t pinset, bool risingedge, bool fallingedge,
            bool event, io_irq_entry func, void *arg, uint32_t priority);

#ifdef cplusplus
}
#endif

#endif
