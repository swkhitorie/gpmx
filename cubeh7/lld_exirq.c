/**
 * low level driver for stm32h7 series, base on cubehal library
 * module external interrupt
*/

#include "include/lld_exirq.h"

lld_exirq_t *mcu_exit_list[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void lld_exit_init(lld_exirq_t *obj, GPIO_TypeDef *port, uint8_t pin, uint32_t mode, uint32_t pull)
{
    lld_gpio_init(&obj->it_pin, port, pin, mode, pull, GPIO_SPEED_FREQ_HIGH, 0);
    obj->it_line = (uint32_t)(0x01 << pin);
    
	IRQn_Type tmpIRQn[16] = {
		EXTI0_IRQn,			EXTI1_IRQn,			EXTI2_IRQn,			EXTI3_IRQn,      /* EXIT IRQ 0~3 */
		EXTI4_IRQn,			EXTI9_5_IRQn,		EXTI9_5_IRQn,		EXTI9_5_IRQn,    /* EXIT IRQ 4~7 */
		EXTI9_5_IRQn,		EXTI9_5_IRQn,		EXTI15_10_IRQn,		EXTI15_10_IRQn,  /* EXIT IRQ 8~11 */
		EXTI15_10_IRQn,		EXTI15_10_IRQn,		EXTI15_10_IRQn,		EXTI15_10_IRQn}; /* EXIT IRQ 12~15 */
	
	HAL_NVIC_SetPriority(tmpIRQn[pin], 2, 0);
	HAL_NVIC_EnableIRQ(tmpIRQn[pin]);
        
    mcu_exit_list[pin] = obj;
}

void lld_exit_irq(lld_exirq_t *obj)
{
	if(lld_gpio_get(&obj->it_pin) && obj->rising) {
        obj->rising();
    } else if (!lld_gpio_get(&obj->it_pin) && obj->falling) {
        obj->falling();
    }   
}

/*****************************************************************
 *****  CubeH7 external interrupt and callback
 ****************************************************************
*/

/**
 * @brief 	External interrupt callback function
 *			It will be called by EXTIx_x_IRQHandler
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
    case GPIO_PIN_0:  lld_exit_irq(mcu_exit_list[0]);  break;
    case GPIO_PIN_1:  lld_exit_irq(mcu_exit_list[1]);  break;
    case GPIO_PIN_2:  lld_exit_irq(mcu_exit_list[2]);  break;
    case GPIO_PIN_3:  lld_exit_irq(mcu_exit_list[3]);  break;
    case GPIO_PIN_4:  lld_exit_irq(mcu_exit_list[4]);  break;
    case GPIO_PIN_5:  lld_exit_irq(mcu_exit_list[5]);  break;
    case GPIO_PIN_6:  lld_exit_irq(mcu_exit_list[6]);  break;
    case GPIO_PIN_7:  lld_exit_irq(mcu_exit_list[7]);  break;
    case GPIO_PIN_8:  lld_exit_irq(mcu_exit_list[8]);  break;
    case GPIO_PIN_9:  lld_exit_irq(mcu_exit_list[9]);  break;
    case GPIO_PIN_10:  lld_exit_irq(mcu_exit_list[10]);  break;
    case GPIO_PIN_11:  lld_exit_irq(mcu_exit_list[11]);  break;
    case GPIO_PIN_12:  lld_exit_irq(mcu_exit_list[12]);  break;
    case GPIO_PIN_13:  lld_exit_irq(mcu_exit_list[13]);  break;
    case GPIO_PIN_14:  lld_exit_irq(mcu_exit_list[14]);  break;
    case GPIO_PIN_15:  lld_exit_irq(mcu_exit_list[15]);  break;
    }
}
/** EXTI0  IRQ */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0); 
}

/** EXTI1  IRQ */
void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

/** EXTI2  IRQ */
void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/** EXTI3  IRQ */
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/** EXTI4  IRQ */
void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4); 
}

/** EXTI5~9  IRQ */
void EXTI9_5_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5); 
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_7) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_8) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_9) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

/** EXTI10~15  IRQ */
void EXTI15_10_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10); 
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET)
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}
