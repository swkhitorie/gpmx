#include "drv_gpio.h"

struct drv_pin_irq_t *drv_external_irq_pin_list[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void drv_gpio_irq_init(struct drv_pin_irq_t *obj, uint32_t priority, void (*entry)())
{
    obj->priority = priority;
    obj->entry = entry;
}

struct drv_pin_t drv_gpio_init(GPIO_TypeDef *port, uint32_t pin, uint32_t mode, 
                uint32_t pull, uint32_t speed, uint32_t alternate, struct drv_pin_irq_t *irq)
{
    struct drv_pin_t obj;
    GPIO_InitTypeDef init_obj;
    IRQn_Type irqn_array[16] = {
        EXTI0_IRQn,	    EXTI1_IRQn,	    EXTI2_IRQn,	    EXTI3_IRQn,      /* EXIT IRQ 0~3 */
        EXTI4_IRQn,	    EXTI9_5_IRQn,   EXTI9_5_IRQn,   EXTI9_5_IRQn,    /* EXIT IRQ 4~7 */
        EXTI9_5_IRQn,   EXTI9_5_IRQn,   EXTI15_10_IRQn,	EXTI15_10_IRQn,  /* EXIT IRQ 8~11 */
        EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn   /* EXIT IRQ 12~15 */
    };

    obj.port = port;
    obj.pin = (uint16_t)(0x01 << pin);
    obj.alternate = alternate;

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

    init_obj.Pin = obj.pin;
    init_obj.Mode = mode;
    init_obj.Pull = pull;
    init_obj.Speed = speed;
    init_obj.Alternate = alternate;
    HAL_GPIO_Init(obj.port, &init_obj);

    if (mode >= IOMODE_IT_RISING && irq != NULL) {
        obj.pin_irq_attr = *irq;
        HAL_NVIC_SetPriority(irqn_array[pin], obj.pin_irq_attr.priority, 0);
        HAL_NVIC_EnableIRQ(irqn_array[pin]);
        drv_external_irq_pin_list[pin] = irq;
    }

    return obj;
}

void drv_gpio_deinit(struct drv_pin_t *obj)
{
    HAL_GPIO_DeInit(obj->port, obj->pin);
}

void drv_gpio_write(struct drv_pin_t *obj, uint8_t val)
{
    HAL_GPIO_WritePin(obj->port, obj->pin, val);
}

uint8_t drv_gpio_read(struct drv_pin_t *obj)
{
    return HAL_GPIO_ReadPin(obj->port, obj->pin);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
    case GPIO_PIN_0:  drv_external_irq_pin_list[0]->entry();  break;
    case GPIO_PIN_1:  drv_external_irq_pin_list[1]->entry();  break;
    case GPIO_PIN_2:  drv_external_irq_pin_list[2]->entry();  break;
    case GPIO_PIN_3:  drv_external_irq_pin_list[3]->entry();  break;
    case GPIO_PIN_4:  drv_external_irq_pin_list[4]->entry();  break;
    case GPIO_PIN_5:  drv_external_irq_pin_list[5]->entry();  break;
    case GPIO_PIN_6:  drv_external_irq_pin_list[6]->entry();  break;
    case GPIO_PIN_7:  drv_external_irq_pin_list[7]->entry();  break;
    case GPIO_PIN_8:  drv_external_irq_pin_list[8]->entry();  break;
    case GPIO_PIN_9:  drv_external_irq_pin_list[9]->entry();  break;
    case GPIO_PIN_10:  drv_external_irq_pin_list[10]->entry();  break;
    case GPIO_PIN_11:  drv_external_irq_pin_list[11]->entry();  break;
    case GPIO_PIN_12:  drv_external_irq_pin_list[12]->entry();  break;
    case GPIO_PIN_13:  drv_external_irq_pin_list[13]->entry();  break;
    case GPIO_PIN_14:  drv_external_irq_pin_list[14]->entry();  break;
    case GPIO_PIN_15:  drv_external_irq_pin_list[15]->entry();  break;
    }
}

void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0); 
}

void EXTI1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4); 
}

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
