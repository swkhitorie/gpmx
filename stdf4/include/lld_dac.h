#ifndef __LLD_DAC_H_
#define __LLD_DAC_H_

/**
 * low level driver for stm32f4 series, base on std periph library
 * module dac
*/

#ifdef __cplusplus
extern "C" {
#endif
    
#include "lld_kernel.h"
#include "lld_gpio.h"

typedef struct __lld_dac {
    uint8_t num;
    lld_gpio_t pin;
    uint32_t channel;
} lld_dac_t;
    
void lld_dac_init(lld_dac_t *obj, uint8_t num);
void lld_dac_set_voltage(lld_dac_t *obj, float value);

#ifdef __cplusplus
}
#endif

#endif
