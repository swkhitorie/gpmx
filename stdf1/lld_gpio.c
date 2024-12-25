/**
 * low level driver for stm32f1 series, base on std periph library
 * module gpio
*/

#include "lld_gpio.h"

void lld_gpio_init(lld_gpio_t *obj, GPIO_TypeDef* port, uint16_t pin, 
    GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed, bool afio)
{
    GPIO_InitTypeDef gpio_struct;
    uint32_t gpio_rcc;

    obj->port = port;
    obj->pin = (uint16_t)(0x01 << pin);  
    obj->afio = afio;
    
	if     (obj->port==GPIOA)	gpio_rcc = RCC_APB2Periph_GPIOA; 
	else if(obj->port==GPIOB)	gpio_rcc = RCC_APB2Periph_GPIOB; 
	else if(obj->port==GPIOC)	gpio_rcc = RCC_APB2Periph_GPIOC; 
	else if(obj->port==GPIOD)	gpio_rcc = RCC_APB2Periph_GPIOD; 
	else if(obj->port==GPIOE)	gpio_rcc = RCC_APB2Periph_GPIOE; 
	else if(obj->port==GPIOF)	gpio_rcc = RCC_APB2Periph_GPIOF; 
	else if(obj->port==GPIOG)	gpio_rcc = RCC_APB2Periph_GPIOG;
	if (afio)                   gpio_rcc |= RCC_APB2Periph_AFIO;
    
    RCC_APB2PeriphClockCmd(gpio_rcc, ENABLE);

	gpio_struct.GPIO_Speed = speed;      
	gpio_struct.GPIO_Mode  = mode;       
	gpio_struct.GPIO_Pin   = obj->pin;    
	GPIO_Init(obj->port, &gpio_struct);
}

uint8_t lld_gpio_get(lld_gpio_t *obj)
{
    return GPIO_ReadInputDataBit(obj->port, obj->pin);
}

void lld_gpio_set(lld_gpio_t *obj, uint8_t value)
{
    if (value) {
        GPIO_SetBits(obj->port, obj->pin); 
    } else {
        GPIO_ResetBits(obj->port, obj->pin); 
    }
}

