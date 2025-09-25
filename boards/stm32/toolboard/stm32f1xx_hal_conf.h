
#ifndef __STM32F1xx_HAL_CONF_H
#define __STM32F1xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined  (HSE_VALUE)
#define HSE_VALUE    8000000U
#endif
#if !defined  (LSE_VALUE)
#define LSE_VALUE    32768U
#endif

#define HSE_STARTUP_TIMEOUT    100U
#define HSI_VALUE              8000000U
#define LSI_VALUE              40000U
#define LSE_STARTUP_TIMEOUT    5000U

/* ########################### System Configuration ######################### */

#define  VDD_VALUE                    3300U /*!< Value of VDD in mv */
#define  TICK_INT_PRIORITY            15U  /*!< tick interrupt priority (lowest by default)*/
#define  USE_RTOS                     0U
#define  PREFETCH_ENABLE              1U

#define  USE_HAL_ADC_REGISTER_CALLBACKS         0U /* ADC register callback disabled  */  
#define  USE_HAL_CAN_REGISTER_CALLBACKS         0U /* CAN register callback disabled  */  
#define  USE_HAL_CEC_REGISTER_CALLBACKS         0U /* CEC register callback disabled  */  
#define  USE_HAL_DAC_REGISTER_CALLBACKS         0U /* DAC register callback disabled  */  
#define  USE_HAL_ETH_REGISTER_CALLBACKS         0U /* ETH register callback disabled */   
#define  USE_HAL_HCD_REGISTER_CALLBACKS         0U /* HCD register callback disabled  */  
#define  USE_HAL_I2C_REGISTER_CALLBACKS         0U /* I2C register callback disabled */   
#define  USE_HAL_I2S_REGISTER_CALLBACKS         0U /* I2S register callback disabled */   
#define  USE_HAL_MMC_REGISTER_CALLBACKS         0U /* MMC register callback disabled  */  
#define  USE_HAL_NAND_REGISTER_CALLBACKS        0U /* NAND register callback disabled */  
#define  USE_HAL_NOR_REGISTER_CALLBACKS         0U /* NOR register callback disabled */   
#define  USE_HAL_PCCARD_REGISTER_CALLBACKS      0U /* PCCARD register callback disabled */
#define  USE_HAL_PCD_REGISTER_CALLBACKS         0U /* PCD register callback disabled */   
#define  USE_HAL_RTC_REGISTER_CALLBACKS         0U /* RTC register callback disabled */   
#define  USE_HAL_SD_REGISTER_CALLBACKS          0U /* SD register callback disabled */    
#define  USE_HAL_SMARTCARD_REGISTER_CALLBACKS   0U /* SMARTCARD register callback disabled */
#define  USE_HAL_IRDA_REGISTER_CALLBACKS        0U /* IRDA register callback disabled */  
#define  USE_HAL_SRAM_REGISTER_CALLBACKS        0U /* SRAM register callback disabled */  
#define  USE_HAL_SPI_REGISTER_CALLBACKS         0U /* SPI register callback disabled */   
#define  USE_HAL_TIM_REGISTER_CALLBACKS         0U /* TIM register callback disabled */   
#define  USE_HAL_UART_REGISTER_CALLBACKS        0U /* UART register callback disabled  */ 
#define  USE_HAL_USART_REGISTER_CALLBACKS       0U /* USART register callback disabled  */
#define  USE_HAL_WWDG_REGISTER_CALLBACKS        0U /* WWDG register callback disabled */  
/* #define USE_FULL_ASSERT    1U */

/* ################## Ethernet peripheral configuration ##################### */
#define MAC_ADDR0   2U
#define MAC_ADDR1   0U
#define MAC_ADDR2   0U
#define MAC_ADDR3   0U
#define MAC_ADDR4   0U
#define MAC_ADDR5   0U

#define ETH_RX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for receive  */          
#define ETH_TX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for transmit */          
#define ETH_RXBUFNB                    8U       /* 4 Rx buffers of size ETH_RX_BUF_SIZE  */
#define ETH_TXBUFNB                    4U       /* 4 Tx buffers of size ETH_TX_BUF_SIZE  */

/* Section 2: PHY configuration section */

/* DP83848_PHY_ADDRESS Address*/
#define DP83848_PHY_ADDRESS             0x01U
/* PHY Reset delay these values are based on a 1 ms Systick interrupt*/
#define PHY_RESET_DELAY                 0x000000FFU
/* PHY Configuration delay */
#define PHY_CONFIG_DELAY                0x00000FFFU

#define PHY_READ_TO                     0x0000FFFFU
#define PHY_WRITE_TO                    0x0000FFFFU

/* Section 3: Common PHY Registers */

#define PHY_BCR                         ((uint16_t)0x00)    /*!< Transceiver Basic Control Register */
#define PHY_BSR                         ((uint16_t)0x01)    /*!< Transceiver Basic Status Register */

#define PHY_RESET                       ((uint16_t)0x8000)  /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000)  /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100)  /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000)  /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100)  /*!< Set the full-duplex mode at 10 Mb/s  */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000)  /*!< Set the half-duplex mode at 10 Mb/s  */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000)  /*!< Enable auto-negotiation function   */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200)  /*!< Restart auto-negotiation function */
#define PHY_POWERDOWN                   ((uint16_t)0x0800)  /*!< Select the power down mode        */
#define PHY_ISOLATE                     ((uint16_t)0x0400)  /*!< Isolate PHY from MII            */  

#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020)  /*!< Auto-Negotiation process completed */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004)  /*!< Valid link established           */ 
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002)  /*!< Jabber condition detected         */

/* Section 4: Extended PHY Registers */
#define PHY_SR                          ((uint16_t)0x10U)    /*!< PHY status register Offset */                  

#define PHY_SPEED_STATUS                ((uint16_t)0x0002U)  /*!< PHY Speed mask  */                             
#define PHY_DUPLEX_STATUS               ((uint16_t)0x0004U)  /*!< PHY Duplex mask */                             

/* ################## SPI peripheral configuration ########################## */
#define USE_SPI_CRC                     0U

#define HAL_MODULE_ENABLED
// #define HAL_ADC_MODULE_ENABLED
// #define HAL_CRYP_MODULE_ENABLED
// #define HAL_CAN_MODULE_ENABLED
//#define HAL_CAN_LEGACY_MODULE_ENABLED
// #define HAL_CEC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
// #define HAL_CRC_MODULE_ENABLED
// #define HAL_DAC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
// #define HAL_ETH_MODULE_ENABLED
// #define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
// #define HAL_I2S_MODULE_ENABLED
// #define HAL_IRDA_MODULE_ENABLED
// #define HAL_IWDG_MODULE_ENABLED
// #define HAL_NOR_MODULE_ENABLED
// #define HAL_NAND_MODULE_ENABLED
// #define HAL_PCCARD_MODULE_ENABLED
// #define HAL_PCD_MODULE_ENABLED
// #define HAL_HCD_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
// #define HAL_RCC_MODULE_ENABLED
#define HAL_RTC_MODULE_ENABLED
// #define HAL_SD_MODULE_ENABLED
// #define HAL_MMC_MODULE_ENABLED
// #define HAL_SDRAM_MODULE_ENABLED
// #define HAL_SMARTCARD_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
// #define HAL_SRAM_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_USART_MODULE_ENABLED
// #define HAL_WWDG_MODULE_ENABLED

#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32f1xx_hal_rcc.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32f1xx_hal_gpio.h"
#endif

#ifdef HAL_EXTI_MODULE_ENABLED
#include "stm32f1xx_hal_exti.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32f1xx_hal_dma.h"
#endif

#ifdef HAL_ETH_MODULE_ENABLED
#include "stm32f1xx_hal_eth.h"
#endif

#ifdef HAL_CAN_MODULE_ENABLED
#include "stm32f1xx_hal_can.h"
#endif

#ifdef HAL_CAN_LEGACY_MODULE_ENABLED
#include "Legacy/stm32f1xx_hal_can_legacy.h"
#endif

#ifdef HAL_CEC_MODULE_ENABLED
#include "stm32f1xx_hal_cec.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32f1xx_hal_cortex.h"
#endif

#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32f1xx_hal_adc.h"
#endif

#ifdef HAL_CRC_MODULE_ENABLED
#include "stm32f1xx_hal_crc.h"
#endif

#ifdef HAL_DAC_MODULE_ENABLED
#include "stm32f1xx_hal_dac.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32f1xx_hal_flash.h"
#endif

#ifdef HAL_SRAM_MODULE_ENABLED
#include "stm32f1xx_hal_sram.h"
#endif

#ifdef HAL_NOR_MODULE_ENABLED
#include "stm32f1xx_hal_nor.h"
#endif

#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32f1xx_hal_i2c.h"
#endif

#ifdef HAL_I2S_MODULE_ENABLED
#include "stm32f1xx_hal_i2s.h"
#endif

#ifdef HAL_IWDG_MODULE_ENABLED
#include "stm32f1xx_hal_iwdg.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32f1xx_hal_pwr.h"
#endif

#ifdef HAL_RTC_MODULE_ENABLED
#include "stm32f1xx_hal_rtc.h"
#endif

#ifdef HAL_PCCARD_MODULE_ENABLED
#include "stm32f1xx_hal_pccard.h"
#endif

#ifdef HAL_SD_MODULE_ENABLED
#include "stm32f1xx_hal_sd.h"
#endif

#ifdef HAL_NAND_MODULE_ENABLED
#include "stm32f1xx_hal_nand.h"
#endif

#ifdef HAL_SPI_MODULE_ENABLED
#include "stm32f1xx_hal_spi.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
#include "stm32f1xx_hal_tim.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
#include "stm32f1xx_hal_uart.h"
#endif

#ifdef HAL_USART_MODULE_ENABLED
#include "stm32f1xx_hal_usart.h"
#endif

#ifdef HAL_IRDA_MODULE_ENABLED
#include "stm32f1xx_hal_irda.h"
#endif

#ifdef HAL_SMARTCARD_MODULE_ENABLED
#include "stm32f1xx_hal_smartcard.h"
#endif

#ifdef HAL_WWDG_MODULE_ENABLED
#include "stm32f1xx_hal_wwdg.h"
#endif

#ifdef HAL_PCD_MODULE_ENABLED
#include "stm32f1xx_hal_pcd.h"
#endif

#ifdef HAL_HCD_MODULE_ENABLED
#include "stm32f1xx_hal_hcd.h"
#endif

#ifdef HAL_MMC_MODULE_ENABLED
#include "stm32f1xx_hal_mmc.h"
#endif

#ifdef  USE_FULL_ASSERT
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t* file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif

