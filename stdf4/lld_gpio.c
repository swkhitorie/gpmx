/**
 * low level driver for stm32f4 series, base on std periph library
 * module gpio
*/

#include "lld_gpio.h"

void lld_gpio_init(lld_gpio_t *obj, GPIO_TypeDef* port, uint16_t pin, 
    GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed,
    uint32_t alternate)
{
    GPIO_InitTypeDef gpio_struct;
    uint32_t gpio_rcc = 0;

    obj->port = port;
    obj->pin = (uint16_t)(0x01 << pin);  
    obj->alternate = alternate;
    
	if     (obj->port==GPIOA)	gpio_rcc = RCC_AHB1Periph_GPIOA; 
	else if(obj->port==GPIOB)	gpio_rcc = RCC_AHB1Periph_GPIOB; 
	else if(obj->port==GPIOC)	gpio_rcc = RCC_AHB1Periph_GPIOC; 
	else if(obj->port==GPIOD)	gpio_rcc = RCC_AHB1Periph_GPIOD; 
	else if(obj->port==GPIOE)	gpio_rcc = RCC_AHB1Periph_GPIOE; 
	else if(obj->port==GPIOF)	gpio_rcc = RCC_AHB1Periph_GPIOF; 
	else if(obj->port==GPIOG)	gpio_rcc = RCC_AHB1Periph_GPIOG;
	else if(obj->port==GPIOH)	gpio_rcc = RCC_AHB1Periph_GPIOH;
	else if(obj->port==GPIOI)	gpio_rcc = RCC_AHB1Periph_GPIOI;
	else if(obj->port==GPIOJ)	gpio_rcc = RCC_AHB1Periph_GPIOJ;
    
    RCC_AHB1PeriphClockCmd(gpio_rcc, ENABLE);
    
    gpio_struct.GPIO_Pin = obj->pin;
	gpio_struct.GPIO_Speed = speed;
	gpio_struct.GPIO_Mode = mode;
	gpio_struct.GPIO_OType = oType;
	gpio_struct.GPIO_PuPd = pupd;
	GPIO_Init(obj->port, &gpio_struct);
	
	if (alternate != 0) GPIO_PinAFConfig(obj->port, (uint8_t)pin, alternate); 
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

