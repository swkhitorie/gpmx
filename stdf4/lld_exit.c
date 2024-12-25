/**
 * low level driver for stm32f4 series, base on std periph library
 * module external interrupt
*/

#include "lld_exit.h"

lld_exit_t *mcu_exit_list[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void lld_exit_init(lld_exit_t *obj, GPIO_TypeDef *port, uint8_t pin, 
    EXTITrigger_TypeDef itmode, GPIOMode_TypeDef mode, uint8_t priority)
{
    lld_gpio_init(&obj->it_pin, port, pin, mode, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, 0);
    obj->it_line = (uint32_t)(0x01 << pin);
    
    uint8_t portsource = 0;
	if     (port==GPIOA)	portsource = EXTI_PortSourceGPIOA; 
	else if(port==GPIOB)   portsource = EXTI_PortSourceGPIOB; 
	else if(port==GPIOC)	portsource = EXTI_PortSourceGPIOC; 
	else if(port==GPIOD)	portsource = EXTI_PortSourceGPIOD;	
	else if(port==GPIOE)	portsource = EXTI_PortSourceGPIOE;	
	else if(port==GPIOF)	portsource = EXTI_PortSourceGPIOF;	
	else if(port==GPIOG)	portsource = EXTI_PortSourceGPIOG;	
	SYSCFG_EXTILineConfig(portsource, pin);
    
	uint8_t tmpIRQn[16] = {EXTI0_IRQn,			EXTI1_IRQn,			EXTI2_IRQn,			EXTI3_IRQn,      
	                       EXTI4_IRQn,			EXTI9_5_IRQn,		EXTI9_5_IRQn,		EXTI9_5_IRQn,    
	                       EXTI9_5_IRQn,		EXTI9_5_IRQn,		EXTI15_10_IRQn,	EXTI15_10_IRQn, 
	                       EXTI15_10_IRQn,	     EXTI15_10_IRQn,	EXTI15_10_IRQn,	EXTI15_10_IRQn}; 
	
	NVIC_InitTypeDef NVIC_InitStructure;                               	
	NVIC_InitStructure.NVIC_IRQChannel = tmpIRQn[pin];	              
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = priority;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            
	NVIC_Init(&NVIC_InitStructure);		
	
	EXTI_InitTypeDef EXTI_InitStructure;							
	EXTI_ClearITPendingBit(obj->it_line);	
	EXTI_InitStructure.EXTI_Line = obj->it_line;					 	 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = itmode;						
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;	        
	EXTI_Init(&EXTI_InitStructure);
    mcu_exit_list[pin] = obj;
}

void lld_exit_irq(lld_exit_t *obj)
{
	EXTI_ClearITPendingBit(obj->it_line);
	
	if(lld_gpio_get(&obj->it_pin) && obj->rising) {
        obj->rising();
    } else if (!lld_gpio_get(&obj->it_pin) && obj->falling) {
        obj->falling();
    }   
}

