#include <board_config.h>

static void board_config_io();

void board_bsp_init()
{
    board_config_io();
    BOARD_LED(false);

#ifdef BSP_MODULE_USB_CHERRY
    HAL_Delay(600);
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
}

void board_debug()
{
    board_led_toggle();
}

void board_led_toggle()
{
	int val = HAL_GPIO_ReadPin(GPIO_nLED_PORT, GPIO_nLED_PIN);
	HAL_GPIO_WritePin(GPIO_nLED_PORT, GPIO_nLED_PIN, !val);
}

void board_config_io()
{
	GPIO_InitTypeDef obj;
    __HAL_RCC_GPIOE_CLK_ENABLE();
    obj.Pin = GPIO_nLED_PIN;
    obj.Mode = GPIO_MODE_OUTPUT_PP;
    obj.Pull = GPIO_NOPULL;
    obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIO_nLED_PORT, &obj);
}
