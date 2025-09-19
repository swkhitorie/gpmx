#include "drv_gpio.h"
#include <string.h>
#define IRQ_LINE_NUM CONFIG_DRV_GPIO_EXTERNAL_IRQ_LINE_NUM
struct gpio_pin_t irq_pin_list[IRQ_LINE_NUM];

struct gpio_pin_t stm32_gpio_setup(
    GPIO_TypeDef *port, uint32_t pin, uint32_t mode, 
    uint32_t pull, uint32_t speed, uint32_t alternate, 
    io_exit_func entry, void *arg, uint32_t priority)
{
    struct gpio_pin_t obj;
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
    obj.priority = priority;

    if (port == GPIOA)			__HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB)		__HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC)		__HAL_RCC_GPIOC_CLK_ENABLE();
#if (BSP_CHIP_RESOURCE_LEVEL > 0)
    else if (port == GPIOD)		__HAL_RCC_GPIOD_CLK_ENABLE();
#if (BSP_CHIP_RESOURCE_LEVEL > 1)
    else if (port == GPIOE)		__HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF)		__HAL_RCC_GPIOF_CLK_ENABLE();
    else if (port == GPIOG)		__HAL_RCC_GPIOG_CLK_ENABLE();
    else if (port == GPIOH)		__HAL_RCC_GPIOH_CLK_ENABLE();
    else if (port == GPIOI)		__HAL_RCC_GPIOI_CLK_ENABLE();
#if (BSP_CHIP_RESOURCE_LEVEL > 2)
    else if (port == GPIOJ)		__HAL_RCC_GPIOJ_CLK_ENABLE();
    else if (port == GPIOK)		__HAL_RCC_GPIOK_CLK_ENABLE();
#endif
#endif
#endif // End With Define BSP_CHIP_RESOURCE_LEVEL

    init_obj.Pin = obj.pin;
    init_obj.Mode = mode;
    init_obj.Pull = pull;
    init_obj.Speed = speed;
#if !defined (DRV_BSP_F1)
    init_obj.Alternate = alternate;
#endif
    HAL_GPIO_Init(obj.port, &init_obj);

    if (mode >= IOMODE_IT_RISING && entry != NULL) {
        obj.callback = entry;
        obj.arg = arg;
        HAL_NVIC_SetPriority(irqn_array[pin], priority, 0);
        HAL_NVIC_EnableIRQ(irqn_array[pin]);
        memcpy(&irq_pin_list[pin], &obj, sizeof(struct gpio_pin_t));
    }

    return obj;
}

void stm32_gpio_write(struct gpio_pin_t *obj, uint8_t val)
{
    HAL_GPIO_WritePin(obj->port, obj->pin, val);
}

uint8_t stm32_gpio_read(struct gpio_pin_t *obj)
{
    return HAL_GPIO_ReadPin(obj->port, obj->pin);
}

bool stm32_gpioread(uint32_t pinset)
{
    GPIO_TypeDef *port = PIN_STPORT(pinset);
    uint16_t pin = PIN_STPIN(pinset);

    return HAL_GPIO_ReadPin(port, 1<<pin);
}

void stm32_gpiowrite(uint32_t pinset, bool value)
{
    GPIO_TypeDef *port = PIN_STPORT(pinset);
    uint16_t pin = PIN_STPIN(pinset);

    HAL_GPIO_WritePin(port, 1<<pin, value);
}

int stm32_gpiosetevent(uint32_t pinset, bool risingedge, bool fallingedge,
                       bool event, io_exit_func func, void *arg, uint32_t priority)
{
    GPIO_TypeDef *port = PIN_STPORT(pinset);
    uint16_t pin = PIN_NO(pinset); //PIN_STPIN(pinset);
    uint32_t mode;
    uint32_t pull;

    if (fallingedge && !risingedge) {

        mode = IOMODE_IT_FALLING;
        pull = IO_PULLUP;
    } else if (!fallingedge && risingedge) {

        mode = IOMODE_IT_RISING;
        pull = IO_PULLDOWN;
    } else if (fallingedge && risingedge) {

        mode = IOMODE_IT_BOTH;
        pull = IO_NOPULL;
    } else {

        // param error
    }

    stm32_gpio_setup(port, pin, mode, pull, IO_SPEEDHIGH, 0, func, arg, priority);

    return 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
    case GPIO_PIN_0:  irq_pin_list[0].callback(irq_pin_list[0].arg);  break;
    case GPIO_PIN_1:  irq_pin_list[1].callback(irq_pin_list[1].arg);  break;
    case GPIO_PIN_2:  irq_pin_list[2].callback(irq_pin_list[2].arg);  break;
    case GPIO_PIN_3:  irq_pin_list[3].callback(irq_pin_list[3].arg);  break;
    case GPIO_PIN_4:  irq_pin_list[4].callback(irq_pin_list[4].arg);  break;
    case GPIO_PIN_5:  irq_pin_list[5].callback(irq_pin_list[5].arg);  break;
    case GPIO_PIN_6:  irq_pin_list[6].callback(irq_pin_list[6].arg);  break;
    case GPIO_PIN_7:  irq_pin_list[7].callback(irq_pin_list[7].arg);  break;
    case GPIO_PIN_8:  irq_pin_list[8].callback(irq_pin_list[8].arg);  break;
    case GPIO_PIN_9:  irq_pin_list[9].callback(irq_pin_list[9].arg);  break;
    case GPIO_PIN_10:  irq_pin_list[10].callback(irq_pin_list[10].arg);  break;
    case GPIO_PIN_11:  irq_pin_list[11].callback(irq_pin_list[11].arg);  break;
    case GPIO_PIN_12:  irq_pin_list[12].callback(irq_pin_list[12].arg);  break;
    case GPIO_PIN_13:  irq_pin_list[13].callback(irq_pin_list[13].arg);  break;
    case GPIO_PIN_14:  irq_pin_list[14].callback(irq_pin_list[14].arg);  break;
    case GPIO_PIN_15:  irq_pin_list[15].callback(irq_pin_list[15].arg);  break;
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
