#include <board_config.h>

CRC_HandleTypeDef hcrc;

void board_crc_init()
{
    __HAL_RCC_CRC_CLK_ENABLE();

    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;

    HAL_CRC_Init(&hcrc);
}

void board_crc_deinit()
{
    __HAL_RCC_CRC_CLK_DISABLE();

    HAL_CRC_DeInit(&hcrc);
}

uint32_t board_crc_key_get(uint32_t *uid, uint32_t key)
{
    uint32_t p[4];
    p[0] = uid[0];
    p[1] = uid[1];
    p[2] = uid[2];
    p[3] = key;

    return HAL_CRC_Calculate(&hcrc, &p[0], 4);
}

