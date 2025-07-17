
#include "radio_board.h"

int32_t BSP_RADIO_Init(void)
{
    GPIO_InitTypeDef  gpio_init_structure = {0};

    /* Enable the Radio Switch Clock */
    RF_SW_CTRL3_GPIO_CLK_ENABLE();

    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin   = RF_SW_CTRL1_PIN;
    gpio_init_structure.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull  = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(RF_SW_CTRL1_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = RF_SW_CTRL2_PIN;
    HAL_GPIO_Init(RF_SW_CTRL2_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = RF_SW_CTRL3_PIN;
    HAL_GPIO_Init(RF_SW_CTRL3_GPIO_PORT, &gpio_init_structure);

    HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET); 

    return BSP_ERROR_NONE;
}

int32_t BSP_RADIO_DeInit(void)
{
    RF_SW_CTRL3_GPIO_CLK_ENABLE();

    /* Turn off switch */
    HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET); 

    /* DeInit the Radio Switch pin */
    HAL_GPIO_DeInit(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN);
    HAL_GPIO_DeInit(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN);
    HAL_GPIO_DeInit(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN);

    return BSP_ERROR_NONE;
}

int32_t BSP_RADIO_ConfigRFSwitch(BSP_RADIO_Switch_TypeDef Config)
{
    switch (Config) {
    case RADIO_SWITCH_OFF:
        {
            /* Turn off switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
            break;      
        }
    case RADIO_SWITCH_RX:
        {
            /*Turns On in Rx Mode the RF Switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_SET); 
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET); 
            break;
        }
    case RADIO_SWITCH_RFO_LP:
        {
            /*Turns On in Tx Low Power the RF Switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_SET); 
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_SET); 
            break;
        }
    case RADIO_SWITCH_RFO_HP:
        {
            /*Turns On in Tx High Power the RF Switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET); 
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_SET); 
            break;
        }
    default:
        break;    
}  

    return BSP_ERROR_NONE;
}

int32_t BSP_RADIO_GetTxConfig(void)
{
    return RADIO_CONF_RFO_LP_HP;
}

int32_t BSP_RADIO_IsTCXO(void)
{
    return RADIO_CONF_TCXO_SUPPORTED;
}

int32_t BSP_RADIO_IsDCDC(void)
{
    return RADIO_CONF_DCDC_SUPPORTED;
}

int32_t BSP_RADIO_GetRFOMaxPowerConfig(BSP_RADIO_RFOMaxPowerConfig_TypeDef Config)
{
    int32_t ret;

    if(Config == RADIO_RFO_LP_MAXPOWER) {
        ret = RADIO_CONF_RFO_LP_MAX_15_dBm;
    } else {
        ret = RADIO_CONF_RFO_HP_MAX_22_dBm;
    }

    return ret;
}

