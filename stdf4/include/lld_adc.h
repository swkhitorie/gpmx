#ifndef __LLD_ADC_H_
#define __LLD_ADC_H_

/**
 * low level driver for stm32f4 series, base on std periph library
 * module adc
*/

#include "lld_kernel.h"
#include "lld_gpio.h"


/*
ADC123
	IN0	IN1	IN2 IN3  IN10 IN11 IN12 IN13
	PA0 PA1 PA2 PA3	 PC0   PC1 PC2  PC3
ADC12
	IN4 IN5 IN6 IN7	IN8 IN9 IN14 IN15
	PA4 PA5 PA6 PA7	PB0 PB1  PC4  PC5
ADC3
	IN_4 IN_5 IN_6 IN_7	IN_8 IN_9 IN_14 IN_15
	PF6	 PF7  PF8  PF9	PF10 PF3  PF4   PF5
*/

typedef struct __lld_adc {
    lld_gpio_t pin;
    float factor;
    uint8_t order_num;
    uint8_t channel_num;
} lld_adc_t;

void lld_adc_init(lld_adc_t *obj, uint8_t ch_num, float k);
float lld_adc_value(lld_adc_t *obj);
#endif
