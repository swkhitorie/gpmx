#ifndef __LLD_EXIRQ_H_
#define __LLD_EXIRQ_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module external interrupt
*/

#include "lld_h7_global.h"
#include "lld_gpio.h"

typedef void (*lld_exit_irq_callback)();

typedef struct __lld_exit {
    uint32_t it_line;
    lld_gpio_t it_pin;
    lld_exit_irq_callback rising;
    lld_exit_irq_callback falling;
} lld_exirq_t;

extern lld_exirq_t *mcu_exit_list[16];

/**
  example: GPIO_MODE_IT_FALLING, GPIO_PULLUP
*/
void lld_exit_init(lld_exirq_t *obj, GPIO_TypeDef *port, uint8_t pin, uint32_t mode, uint32_t pull);

void lld_exit_irq(lld_exirq_t *obj);


#ifdef __cplusplus
}
#endif

#endif
