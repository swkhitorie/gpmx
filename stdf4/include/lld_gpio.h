#ifndef __LLD_GPIO_H_
#define __LLD_GPIO_H_

/**
 * low level driver for stm32f4 series, base on std periph library
 * module gpio
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "lld_kernel.h"

typedef struct __lld_gpio {
	GPIO_TypeDef *port;   
    uint16_t pin;
    uint32_t alternate;
} lld_gpio_t;

/**
  GPIO_Mode_IN/GPIO_Mode_OUT/GPIO_Mode_AF/GPIO_Mode_AN
  GPIO_OType_PP/GPIO_OType_OD
  GPIO_Low_Speed/GPIO_Medium_Speed/GPIO_Fast_Speed/GPIO_High_Speed
  GPIO_PuPd_NOPULL/GPIO_PuPd_UP/GPIO_PuPd_DOWN
*/
void lld_gpio_init(lld_gpio_t *obj, GPIO_TypeDef* port, uint16_t pin, 
    GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed,
    uint32_t alternate);

void lld_gpio_set(lld_gpio_t *obj, uint8_t value);
uint8_t lld_gpio_get(lld_gpio_t *obj);

#ifdef __cplusplus
}
#endif

#endif
