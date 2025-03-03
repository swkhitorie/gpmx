#ifndef DRV_UART_H_
#define DRV_UART_H_

#include <stdint.h>
#include "gd32c10x.h"

#include "drv_gpio.h"
#include "drv_buffer.h"

enum uart_wordlen {
    WL_8BIT = USART_WL_8BIT,
    WL_9BIT = USART_WL_9BIT
};

enum uart_stopbit_len {
    STB_0_5BIT = USART_STB_0_5BIT,
    STB_1BIT = USART_STB_1BIT,
    STB_1_5BIT = USART_STB_1_5BIT,
    STB_2BIT = USART_STB_2BIT
};

enum uart_parity {
    PARITY_NONE = USART_PM_NONE,
    PARITY_EVEN = USART_PM_EVEN,
    PARITY_ODD = USART_PM_ODD
};

struct drv_uart_attr_t {
    uint32_t baudrate;
    uint32_t wordlen;
    uint32_t stopbitlen;
    uint32_t parity;
    uint8_t priority;
};

struct drv_uart_dma_attr_t {
    uint8_t *mem_buff;
    uint32_t mem_capacity;
    uint8_t priority;
};

struct drv_uart_t
{
    uint32_t com;

    uint32_t txdma_periph;
    uint32_t rxdma_periph;
    dma_channel_enum tx_channel;
    dma_channel_enum rx_channel;

    struct drv_uart_attr_t attr;
    struct drv_uart_dma_attr_t attr_txdma;
    struct drv_uart_dma_attr_t attr_rxdma;
    
    devbuf_t tx_buf;
    devbuf_t rx_buf;
    
    bool tx_busy;
};

extern struct drv_uart_t *drv_uart_list[5];

#ifdef cplusplus
  extern "C" {
#endif

void drv_uart_dma_attr_init(struct drv_uart_dma_attr_t *obj, 
                            uint8_t *p, uint32_t len,
                            uint8_t priority);

void drv_uart_attr_init(struct drv_uart_attr_t *obj, 
                        uint32_t baudrate, uint32_t wordlen,
                        uint32_t stopbitlen, uint32_t parity,
                        uint8_t priority);

void drv_uart_buff_init(struct drv_uart_t *obj, uint8_t *txbuf, uint16_t tlen,
                        uint8_t *rxbuf, uint16_t rlen);

void drv_uart_init(uint8_t num, struct drv_uart_t *obj,
                   struct drv_uart_attr_t *com_attr,
                   struct drv_uart_dma_attr_t *txdma_attr,
                   struct drv_uart_dma_attr_t *rxdma_attr);

int drv_uart_send(struct drv_uart_t *obj, const uint8_t *p, uint16_t len, uint8_t way);

void drv_uart_irq(struct drv_uart_t *obj);

void drv_uart_txdma_irq(struct drv_uart_t *obj);

void drv_uart_rxdma_irq(struct drv_uart_t *obj);

#ifdef cplusplus
}
#endif


#endif

