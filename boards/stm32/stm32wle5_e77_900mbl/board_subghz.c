#include "board_config.h"

SUBGHZ_HandleTypeDef hsubghz;

void board_subghz_init()
{
    int ret;
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
    UTIL_TIMER_Init();

    hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
    ret = HAL_SUBGHZ_Init(&hsubghz);
    if (ret != HAL_OK) {
        while(1){}
    }
    __HAL_RCC_SUBGHZSPI_CLK_ENABLE();
    HAL_NVIC_SetPriority(SUBGHZ_Radio_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
}

bool board_subghz_tx_ready()
{
    return (READ_BIT(SUBGHZSPI->SR, SPI_SR_TXE) == (SPI_SR_TXE));
}

/****************************************************************************
 * Low Layer
 ****************************************************************************/
void SUBGHZ_Radio_IRQHandler(void)
{
    HAL_SUBGHZ_IRQHandler(&hsubghz);
}
