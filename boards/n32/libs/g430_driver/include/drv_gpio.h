#ifndef DRV_GPIO_H_
#define DRV_GPIO_H_

#include "n32g430.h"

#define LOW_INITPIN(port, pin, mode, pull, speed) \
        n32_gpio_init(port, pin, mode, pull, speed, 0)

#define LOW_PERIPH_INITPIN(port, pin, mode, pull, speed, alternate) \
        n32_gpio_init(port, pin, mode, pull, speed, alternate)

struct io_pin_t {
    GPIO_Module *port;
    uint16_t pin;
};

struct periph_pin_t {
    GPIO_Module *port;
    uint16_t pin;
    uint32_t alternate;
};

#ifdef __cplusplus
extern "C" {
#endif

void n32_gpio_init(GPIO_Module* port, uint8_t pin, uint32_t mode,
    uint32_t pull, uint32_t speed, uint32_t alternate);

void n32_pin_set(GPIO_Module* port, uint8_t pin, uint8_t val);

uint8_t n32_pin_out_get(GPIO_Module* port, uint8_t pin);

uint8_t n32_pin_in_get(GPIO_Module* port, uint8_t pin);

void n32_pin_toggle(GPIO_Module* port, uint8_t pin);

#ifdef __cplusplus
}
#endif



#endif
