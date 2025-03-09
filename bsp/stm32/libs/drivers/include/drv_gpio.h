#ifndef DRV_GPIO_H_
#define DRV_GPIO_H_

#include <stdint.h>
#include "drv_common.h"

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

struct gpio_pin_t {
    GPIO_TypeDef *port;   
    uint16_t pin;
    uint32_t alternate;

    void (*entry)();
    uint32_t priority;
};

#ifdef cplusplus
extern "C" {
#endif

struct gpio_pin_t low_gpio_setup(
    GPIO_TypeDef *port, uint32_t pin, uint32_t mode, 
    uint32_t pull, uint32_t speed, uint32_t alternate, 
    void (*entry)(), uint32_t priority);

void low_gpio_write(struct gpio_pin_t *obj, uint8_t val);

uint8_t low_gpio_read(struct gpio_pin_t *obj);


#ifdef cplusplus
}
#endif

#endif
