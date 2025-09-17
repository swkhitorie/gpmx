#ifndef DRV_UART_H_
#define DRV_UART_H_

#include "n32g430.h"
#include <drv_gpio.h>
#include <device/rbdrv.h>

struct n32_uart_dev {

    uint8_t id;
    uint32_t baud;
    uint32_t wordlen;
    uint32_t stopbits;
    uint32_t parity;
    
    uint8_t priority;
    
    DMA_ChannelType  *rxdma_channel;
    uint8_t          *rxdma_buffer;
    uint16_t         rxdma_size;

    struct rbdrv rxbuf;

    GPIO_Module   *txport;
    uint8_t       txpin;
    uint32_t      txalternate;

    GPIO_Module   *rxport;
    uint8_t       rxpin;
    uint32_t      rxalternate;
};

#ifdef __cplusplus
extern "C" {
#endif

void n32_uart_init(struct n32_uart_dev *dev);

uint16_t n32_uart_sendblk(struct n32_uart_dev *dev, uint8_t *p, uint16_t size);
    
uint16_t n32_uart_dmasend(struct n32_uart_dev *dev, uint8_t *p, uint16_t size);

#ifdef __cplusplus
}
#endif



#endif
