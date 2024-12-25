#ifndef __LLD_EXIT_H_
#define __LLD_EXIT_H_

/**
 * low level driver for stm32f4 series, base on std periph library
 * module external interrupt
*/

#include "lld_kernel.h"
#include "lld_gpio.h"


typedef void (*lld_exit_irq_callback)();

typedef struct __lld_exit {
    uint32_t it_line;
    lld_gpio_t it_pin;
    lld_exit_irq_callback rising;
    lld_exit_irq_callback falling;
} lld_exit_t;

extern lld_exit_t *mcu_exit_list[16];

void lld_exit_init(lld_exit_t *obj, GPIO_TypeDef *port, uint8_t pin, 
    EXTITrigger_TypeDef itmode, GPIOMode_TypeDef mode, uint8_t priority);

void lld_exit_irq(lld_exit_t *obj);
#endif
