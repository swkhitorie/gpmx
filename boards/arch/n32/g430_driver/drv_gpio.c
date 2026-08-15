#include "drv_gpio.h"

void n32_gpio_init(GPIO_Module* port, uint8_t pin, uint32_t mode,
    uint32_t pull, uint32_t speed, uint32_t alternate)
{
    GPIO_InitType io_init;
    GPIO_Structure_Initialize(&io_init);

    if (port == GPIOA) {
        RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_GPIOA);
    } else if(port == GPIOB) {
        RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_GPIOB);
    } else if(port == GPIOC) {
        RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_GPIOC);
    } else {
        RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_GPIOD);
    }

    io_init.Pin            = ((uint16_t)0x01<<pin);    
    io_init.GPIO_Mode      = mode;
    io_init.GPIO_Pull      = pull;
    io_init.GPIO_Slew_Rate = speed;
    io_init.GPIO_Alternate = alternate;
    GPIO_Peripheral_Initialize(port, &io_init);
}

void n32_pin_set(GPIO_Module* port, uint8_t pin, uint8_t val)
{
    if (val == 0) {
        GPIO_Pins_Reset(port, ((uint16_t)0x01<<pin));
    } else {
        GPIO_Pins_Set(port, ((uint16_t)0x01<<pin));
    }
}

uint8_t n32_pin_out_get(GPIO_Module* port, uint8_t pin)
{
    return GPIO_Output_Pin_Data_Get(port, ((uint16_t)0x01<<pin)); 
}

uint8_t n32_pin_in_get(GPIO_Module* port, uint8_t pin)
{
    return GPIO_Input_Pin_Data_Get(port, ((uint16_t)0x01<<pin));
}

void n32_pin_toggle(GPIO_Module* port, uint8_t pin)
{
    GPIO_Pin_Toggle(port, ((uint16_t)0x01<<pin));
}

