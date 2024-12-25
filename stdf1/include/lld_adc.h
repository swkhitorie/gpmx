#ifndef __LLD_ADC_H_
#define __LLD_ADC_H_

/**
 * low level driver for stm32f1 series, base on std periph library
 * module adc
*/

#include "lld_kernel.h"
#include "lld_gpio.h"

typedef struct __lld_adc {
    lld_gpio_t pin;
    float factor;
    uint8_t order_num;
    uint8_t channel_num;
} lld_adc_t;

void lld_adc_init(lld_adc_t *obj, uint8_t ch_num, float k);
float lld_adc_value(lld_adc_t *obj);
#endif
