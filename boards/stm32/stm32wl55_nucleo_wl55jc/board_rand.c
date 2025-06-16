#include <board_config.h>

static RNG_HandleTypeDef hrng;
void board_rng_init()
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    hrng.Instance = RNG;
    hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
    PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_MSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        while(1){}
    }
    __HAL_RCC_RNG_CLK_ENABLE();

    if (HAL_RNG_Init(&hrng) != HAL_OK) {
        while(1){}
    }
}

uint32_t board_rng_get()
{
    uint32_t rng_val;
    HAL_RNG_GenerateRandomNumber(&hrng, &rng_val);
    return rng_val;
}
