#include "board_config.h"

void board_usb_init()
{
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
}

void usb_dc_low_level_init(uint8_t busid)
{
	/* select usb clock -> PLL3Q 48M, from HSE */
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;

    /* USB Clock Need 48MHZ, Select Source: PLL3Q */
    /* PLL3Q(48MHz) =  HSE(16M)/PLL3M*PLL3N/PLL3Q */
    PeriphClkInitStruct.PLL3.PLL3M = 1;
    PeriphClkInitStruct.PLL3.PLL3N = 6;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    while (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {}

    HAL_PWREx_EnableUSBVoltageDetector();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

void usb_dc_low_level_deinit(uint8_t busid)
{
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
}

void OTG_FS_IRQHandler(void)
{
    extern void USBD_IRQHandler(uint8_t busid);
    USBD_IRQHandler(0);
}
