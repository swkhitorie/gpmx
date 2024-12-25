#ifndef __LLD_GPIO_H_
#define __LLD_GPIO_H_

/**
 * low level driver for stm32f1 series, base on std periph library
 * module gpio
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "lld_kernel.h"

typedef struct __lld_gpio {
	GPIO_TypeDef *port;   
    uint16_t pin;
    bool afio;
} lld_gpio_t;

/**
  GPIO_Speed_2MHz/GPIO_Speed_10MHz/GPIO_Speed_50MHz

  GPIO_Mode_AIN/GPIO_Mode_IN_FLOATING
  GPIO_Mode_IPD/GPIO_Mode_IPU
  GPIO_Mode_Out_OD/GPIO_Mode_Out_PP
  GPIO_Mode_AF_OD/GPIO_Mode_AF_PP
*/
void lld_gpio_init(lld_gpio_t *obj, GPIO_TypeDef* port, uint16_t pin, 
    GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed, bool afio);

void lld_gpio_set(lld_gpio_t *obj, uint8_t value);
uint8_t lld_gpio_get(lld_gpio_t *obj);

#ifdef __cplusplus
}
#endif

#endif
