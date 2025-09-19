#ifndef DRIVE_UART_H_
#define DRIVE_UART_H_

#include <stdint.h>
#include "drv_cmn.h"
#include "drv_gpio.h"
#include "drv_dma.h"
#include <device/serial.h>

#if defined(DRV_STM32_WL) || defined(DRV_STM32_H7)

#define UART_SET_TDR(__HANDLE__, __DATA__)  ((__HANDLE__)->Instance->TDR = (__DATA__))
#define UART_GET_RDR(__HANDLE__, MASK)            ((__HANDLE__)->Instance->RDR & MASK)
#else

#define UART_SET_TDR(__HANDLE__, __DATA__)  ((__HANDLE__)->Instance->DR = (__DATA__))
#define UART_GET_RDR(__HANDLE__, MASK)            ((__HANDLE__)->Instance->DR & MASK)
#endif

#define DEVICE_USART_DMA_TX  (0x01)
#define DEVICE_USART_DMA_RX  (0x02)

struct up_uart_dev_s
{
    struct uart_dev_s dev;   /* Generic UART device */

    uint8_t id;
    struct periph_pin_t txpin;
    struct periph_pin_t rxpin;
    struct dma_config   txdma_cfg;
    struct dma_config   rxdma_cfg;
    uint8_t priority;

    UART_HandleTypeDef com;
    DMA_HandleTypeDef txdma;
    DMA_HandleTypeDef rxdma;
};

extern const struct uart_ops_s g_uart_ops;

extern struct uart_dev_s *g_uart_list[CONFIG_STM32_UART_NUM];

#define STM32_USART_DMA_TX_IRQ(num)  \
    struct up_uart_dev_s *priv = g_uart_list[num-1]->priv; \
    HAL_DMA_IRQHandler(priv->com.hdmatx)

#define STM32_USART_DMA_RX_IRQ(num)  \
    struct up_uart_dev_s *priv = g_uart_list[num-1]->priv; \
    HAL_DMA_IRQHandler(priv->com.hdmarx)

#endif

