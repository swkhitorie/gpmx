/**
 * low level driver for stm32h7 series, base on cubehal library
 * module gpio
*/

#include "include/lld_gpio.h"

void lld_gpio_init(lld_gpio_t *obj, GPIO_TypeDef *port, uint16_t pin, 
    uint32_t mode, uint32_t pull, uint32_t speed, uint32_t alternate)
{
    GPIO_InitTypeDef gpio_init;
	obj->port = port;             
	obj->pin  = (uint16_t)(1 << pin);   
	
	if (port == GPIOA)			__HAL_RCC_GPIOA_CLK_ENABLE();
	else if (port == GPIOB)		__HAL_RCC_GPIOB_CLK_ENABLE();
	else if (port == GPIOC)		__HAL_RCC_GPIOC_CLK_ENABLE();
	else if (port == GPIOD)		__HAL_RCC_GPIOD_CLK_ENABLE();
	else if (port == GPIOE)		__HAL_RCC_GPIOE_CLK_ENABLE();
	else if (port == GPIOF)		__HAL_RCC_GPIOF_CLK_ENABLE();
	else if (port == GPIOG)		__HAL_RCC_GPIOG_CLK_ENABLE();
	else if (port == GPIOH)		__HAL_RCC_GPIOH_CLK_ENABLE();
	else if (port == GPIOI)		__HAL_RCC_GPIOI_CLK_ENABLE();
	else if (port == GPIOJ)		__HAL_RCC_GPIOJ_CLK_ENABLE();
	else if (port == GPIOK)		__HAL_RCC_GPIOK_CLK_ENABLE();
	
	gpio_init.Pin = obj->pin;
	gpio_init.Mode = mode;
	gpio_init.Pull = pull;
	gpio_init.Speed = speed;
	gpio_init.Alternate = alternate;
	HAL_GPIO_Init(obj->port, &gpio_init);
}

void lld_gpio_set(lld_gpio_t *obj, uint8_t value)
{
	if (value) {
        HAL_GPIO_WritePin(obj->port, obj->pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(obj->port, obj->pin, GPIO_PIN_RESET);
    }			
}

uint8_t lld_gpio_get(lld_gpio_t *obj)
{
	return HAL_GPIO_ReadPin(obj->port, obj->pin);
}
