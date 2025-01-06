#ifndef __LLD_GPIO_H_
#define __LLD_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module gpio
*/

#include "lld_h7_global.h"

typedef struct __lld_gpio {
	GPIO_TypeDef *port;   
    uint16_t pin;
    uint32_t alternate;
} lld_gpio_t;

/**
  GPIO_NOPULL/GPIO_PULLUP/GPIO_PULLDOWN
  GPIO_SPEED_FREQ_LOW/GPIO_SPEED_FREQ_MEDIUM/GPIO_SPEED_FREQ_HIGH/GPIO_SPEED_FREQ_VERY_HIGH
  GPIO_MODE_INPUT/GPIO_MODE_OUTPUT_PP/GPIO_MODE_OUTPUT_OD
  GPIO_MODE_AF_PP/GPIO_MODE_AF_OD/GPIO_MODE_ANALOG 
  GPIO_MODE_IT_RISING/GPIO_MODE_IT_FALLING/GPIO_MODE_IT_RISING_FALLING
*/
void lld_gpio_init(lld_gpio_t *obj, GPIO_TypeDef *port, uint16_t pin, 
    uint32_t mode, uint32_t pull, uint32_t speed, uint32_t alternate);

void lld_gpio_set(lld_gpio_t *obj, uint8_t value);
uint8_t lld_gpio_get(lld_gpio_t *obj);

    

#ifdef __cplusplus
}
#endif

#endif
