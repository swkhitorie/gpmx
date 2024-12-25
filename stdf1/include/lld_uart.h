#ifndef __LLD_UART_H_
#define __LLD_UART_H_

/**
 * low level driver for stm32f1 series, base on std periph library
 * module uart
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "lld_kernel.h"
#include "lld_gpio.h"
#include "devfifobuffer.h"

#define USART_DMA_TX_BUFFER_SIZE (300)
#define USART_DMA_RX_BUFFER_SIZE (300)

typedef struct __lld_uart_dma {
    DMA_InitTypeDef init;
    uint8_t *buffer;
    DMA_Channel_TypeDef *channel;
    uint32_t flag_tc;
    uint32_t it_gl;
} lld_uart_dma_t;
    
typedef struct __lld_uart {
    USART_TypeDef *huart;
    uint8_t com;
    uint32_t baud;
    uint16_t parity;
    uint16_t wordlen;
    uint16_t stopbits;
    lld_gpio_t tx_pin;
    lld_gpio_t rx_pin;
    devfifobuffer_t txbuf;
    devfifobuffer_t rxbuf;
    bool txdma_enable;
    bool tx_busy;
    bool rxdma_enable;
    lld_uart_dma_t txdma;
    lld_uart_dma_t rxdma;
} lld_uart_t;

extern lld_uart_t *mcu_uart_list[5];

/**
    USART_Parity_No
    USART_WordLength_8b
    USART_StopBits_1
*/
void lld_uart_init(lld_uart_t *obj, uint8_t com, uint32_t baud, 
        bool remap, bool txdma_en,bool rxdma_en,
        uint16_t parity, uint16_t wordlen, uint16_t stopbits);
void lld_uart_tx_dma_init(lld_uart_t *obj);
void lld_uart_rx_dma_init(lld_uart_t *obj);  
bool lld_uart_send_bytes(lld_uart_t *obj, const uint8_t *p, uint16_t size, lld_rwway way);

void lld_uart_irq(lld_uart_t *obj);
void lld_uart_irq_txdma(lld_uart_t *obj);
void lld_uart_irq_rxdma(lld_uart_t *obj);

#ifdef __cplusplus
}
#endif

#endif
