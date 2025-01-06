#ifndef __LLD_UART_H_
#define __LLD_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module uart
*/

#include "lld_h7_global.h"
#include "lld_gpio.h"
#include <lld_utils.h>

#define USART_DMA_TX_BUFFER_SIZE (256)
#define USART_DMA_RX_BUFFER_SIZE (256)

typedef struct __lld_uart_dma {
    DMA_HandleTypeDef handle;
    uint8_t *buffer;
} lld_uart_dma_t;
    
typedef struct __lld_uart {
    UART_HandleTypeDef huart;
    uint8_t com;
    uint32_t baud;
    uint16_t parity;
    uint16_t wordlen;
    uint16_t stopbits;
    lld_gpio_t tx_pin;
    lld_gpio_t rx_pin;
    devbuf_t txbuf;
    devbuf_t rxbuf;
    bool txdma_enable;
    bool tx_busy;
    bool rxdma_enable;
    lld_uart_dma_t txdma;
    lld_uart_dma_t rxdma;
} lld_uart_t;

extern lld_uart_t *mcu_uart_list[8];

/**
    UART_PARITY_NONE / UART_PARITY_EVEN / UART_PARITY_ODD
    UART_WORDLENGTH_7B / UART_WORDLENGTH_8B / UART_WORDLENGTH_9B
    UART_STOPBITS_0_5 / UART_STOPBITS_1 / UART_STOPBITS_1_5 / UART_STOPBITS_2
*/
void lld_uart_init(lld_uart_t *obj, uint8_t com, uint32_t baud, 
        uint8_t tx_selec, uint8_t rx_selec, bool txdma_en, bool rxdma_en,
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
