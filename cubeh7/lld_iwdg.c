/**
 * low level driver for stm32h7 series, base on cubehal library
 * module independent watch dog
*/

#include "include/lld_iwdg.h"

bool is_init = false;
const uint16_t reload_max = 4095;
static IWDG_HandleTypeDef IWDG_Handler;

void lld_iwdg_init(lld_iwdt_presc_t _pre, uint16_t reload)
{
	if (reload > reload_max)
		reload = reload_max;
	IWDG_Handler.Instance = IWDG1;
	IWDG_Handler.Init.Prescaler = _pre;   
	IWDG_Handler.Init.Reload = reload;       
	IWDG_Handler.Init.Window = IWDG_WINDOW_DISABLE;
	HAL_IWDG_Init(&IWDG_Handler); 
	is_init = true;
}

void lld_iwdg_feed()
{
	if (!is_init) lld_iwdg_init(PRE_64, 1000);
	HAL_IWDG_Refresh(&IWDG_Handler);
}
