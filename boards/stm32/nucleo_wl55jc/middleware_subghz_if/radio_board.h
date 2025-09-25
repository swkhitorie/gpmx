#ifndef BOARD_RADIO_H_
#define BOARD_RADIO_H_

#include <stdint.h>
#include <stm32wlxx_hal.h>

#define BSP_ERROR_NONE                         0
#define BSP_ERROR_NO_INIT                     -1
#define BSP_ERROR_WRONG_PARAM                 -2
#define BSP_ERROR_BUSY                        -3
#define BSP_ERROR_PERIPH_FAILURE              -4
#define BSP_ERROR_COMPONENT_FAILURE           -5
#define BSP_ERROR_UNKNOWN_FAILURE             -6
#define BSP_ERROR_UNKNOWN_COMPONENT           -7
#define BSP_ERROR_BUS_FAILURE                 -8
#define BSP_ERROR_CLOCK_FAILURE               -9
#define BSP_ERROR_MSP_FAILURE                 -10
#define BSP_ERROR_FEATURE_NOT_SUPPORTED       -11

#define RADIO_CONF_RFO_LP_HP                     0U
#define RADIO_CONF_RFO_LP                        1U
#define RADIO_CONF_RFO_HP                        2U

#define RADIO_CONF_TCXO_NOT_SUPPORTED            0U
#define RADIO_CONF_TCXO_SUPPORTED                1U

#define RADIO_CONF_DCDC_NOT_SUPPORTED            0U
#define RADIO_CONF_DCDC_SUPPORTED                1U

#define RADIO_CONF_RFO_HP_MAX_22_dBm  ((int32_t) 22)
#define RADIO_CONF_RFO_HP_MAX_20_dBm  ((int32_t) 20)
#define RADIO_CONF_RFO_HP_MAX_17_dBm  ((int32_t) 17)
#define RADIO_CONF_RFO_HP_MAX_14_dBm  ((int32_t) 14)
#define RADIO_CONF_RFO_LP_MAX_15_dBm  ((int32_t) 15)
#define RADIO_CONF_RFO_LP_MAX_14_dBm  ((int32_t) 14)
#define RADIO_CONF_RFO_LP_MAX_10_dBm  ((int32_t) 10)

#define RF_SW_CTRL3_PIN                          GPIO_PIN_3
#define RF_SW_CTRL3_GPIO_PORT                    GPIOC
#define RF_SW_CTRL3_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOC_CLK_ENABLE()
#define RF_SW_CTRL3_GPIO_CLK_DISABLE()           __HAL_RCC_GPIOC_CLK_DISABLE()

#define RF_SW_CTRL1_PIN                          GPIO_PIN_4
#define RF_SW_CTRL1_GPIO_PORT                    GPIOC
#define RF_SW_CTRL1_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOC_CLK_ENABLE()
#define RF_SW_RX_GPIO_CLK_DISABLE()              __HAL_RCC_GPIOC_CLK_DISABLE()

#define RF_SW_CTRL2_PIN                          GPIO_PIN_5
#define RF_SW_CTRL2_GPIO_PORT                    GPIOC
#define RF_SW_CTRL2_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOC_CLK_ENABLE()
#define RF_SW_CTRL2_GPIO_CLK_DISABLE()           __HAL_RCC_GPIOC_CLK_DISABLE()

#define RF_TCXO_VCC_PIN                          GPIO_PIN_0
#define RF_TCXO_VCC_GPIO_PORT                    GPIOB
#define RF_TCXO_VCC_CLK_ENABLE()                 __HAL_RCC_GPIOB_CLK_ENABLE()
#define RF_TCXO_VCC_CLK_DISABLE()                __HAL_RCC_GPIOB_CLK_DISABLE()

typedef enum 
{
    RADIO_SWITCH_OFF    = 0,
    RADIO_SWITCH_RX     = 1,
    RADIO_SWITCH_RFO_LP = 2,
    RADIO_SWITCH_RFO_HP = 3,
}BSP_RADIO_Switch_TypeDef;

typedef enum
{
    RADIO_RFO_LP_MAXPOWER = 0,
    RADIO_RFO_HP_MAXPOWER,
} BSP_RADIO_RFOMaxPowerConfig_TypeDef;

#ifdef __cplusplus
extern "C" {
#endif

int32_t BSP_RADIO_Init(void);
int32_t BSP_RADIO_DeInit(void);
int32_t BSP_RADIO_ConfigRFSwitch(BSP_RADIO_Switch_TypeDef Config);
int32_t BSP_RADIO_GetTxConfig(void);
int32_t BSP_RADIO_IsTCXO(void);
int32_t BSP_RADIO_IsDCDC(void);
int32_t BSP_RADIO_GetRFOMaxPowerConfig(BSP_RADIO_RFOMaxPowerConfig_TypeDef Config);

#ifdef __cplusplus
}
#endif

#endif
