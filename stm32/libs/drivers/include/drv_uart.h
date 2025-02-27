#ifndef DRV_UART_H_
#define DRV_UART_H_

#include <stdint.h>
#include "drv_common.h"
#include "drv_gpio.h"
#include "drv_buffer.h"

#if defined (DRV_BSP_H7)
#include "drv_globalpin_h7.h"
#endif

#define DRV_UART_PERIPHAL_NUM   CONFIG_UART_PERIPHAL_NUM

enum uart_wordlen {
#if !defined (DRV_BSP_F1) && !defined (DRV_BSP_F4)
    WL_7BIT = UART_WORDLENGTH_7B,
#endif
    WL_8BIT = UART_WORDLENGTH_8B,
    WL_9BIT = UART_WORDLENGTH_9B,
};

enum uart_stopbit_len {
#if !defined (DRV_BSP_F1) && !defined (DRV_BSP_F4)
    STB_0_5BIT = UART_STOPBITS_0_5,
    STB_1_5BIT = UART_STOPBITS_1_5,
#endif
    STB_1BIT = UART_STOPBITS_1,
    STB_2BIT = UART_STOPBITS_2,
};

enum uart_parity {
    PARITY_NONE = UART_PARITY_NONE,
    PARITY_EVEN = UART_PARITY_EVEN,
    PARITY_ODD = UART_PARITY_ODD,
};

#pragma pack (4)
struct drv_uart_attr_t {
    uint32_t baudrate;
    uint32_t wordlen;
    uint32_t stopbitlen;
    uint32_t parity;
    uint8_t priority;
};

struct drv_uart_dma_attr_t {
    uint16_t mem_capacity;
    uint16_t mem_halfcapacity;
    uint8_t mem_halflag;
    uint8_t *mem_buff;
    uint8_t priority;
};

struct drv_uart_t
{
    UART_HandleTypeDef com;

    DMA_HandleTypeDef txdma;
    DMA_HandleTypeDef rxdma;

    struct drv_uart_attr_t attr;
    struct drv_uart_dma_attr_t attr_txdma;
    struct drv_uart_dma_attr_t attr_rxdma;

    devbuf_t tx_buf;
    devbuf_t rx_buf;

    bool tx_busy;
};
#pragma pack (0)

extern struct drv_uart_t *drv_uart_list[DRV_UART_PERIPHAL_NUM];

#ifdef cplusplus
extern "C" {
#endif

void drv_uart_dma_attr_init(struct drv_uart_dma_attr_t *obj, 
                            uint8_t *p, uint16_t len,
                            uint8_t priority);

void drv_uart_attr_init(struct drv_uart_attr_t *obj, 
                        uint32_t baudrate, uint32_t wordlen,
                        uint32_t stopbitlen, uint32_t parity,
                        uint8_t priority);

void drv_uart_init(uint8_t num, struct drv_uart_t *obj,
                    uint8_t tx_selc, uint8_t rx_selc,
                    struct drv_uart_attr_t *com_attr,
                    struct drv_uart_dma_attr_t *txdma_attr,
                    struct drv_uart_dma_attr_t *rxdma_attr);

void drv_uart_buff_init(struct drv_uart_t *obj, uint8_t *txbuf, uint16_t tlen,
                        uint8_t *rxbuf, uint16_t rlen);

void drv_uart_irq(struct drv_uart_t *obj);

void drv_uart_txdma_irq(struct drv_uart_t *obj);

void drv_uart_rxdma_irq(struct drv_uart_t *obj);

/* operation need thread safe */
int drv_uart_send(struct drv_uart_t *obj, const uint8_t *p, uint16_t len, enum __drv_rwway way);

/* operation need thread safe */
devbuf_t drv_uart_devbuf(struct drv_uart_t *obj);

#ifdef cplusplus
}
#endif


#endif

