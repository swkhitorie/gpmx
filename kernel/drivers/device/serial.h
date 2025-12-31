#ifndef DEV_OPS_SERIAL_H_
#define DEV_OPS_SERIAL_H_

#ifdef CONFIG_GPDRIVE_SERIAL

#include "dnode.h"

#define SERIAL_DMASEND(d,m,c) ((d)->ops->dmasend(d,m,c))

#define SERIAL_SEND(d,m,c) ((d)->ops->send(d,m,c))

#define SERIAL_RDBUF(d,m,c) ((d)->ops->readbuf(d,m,c))

#define SERIAL_RXCLEAR(d) ((d)->ops->rxclear(d))

struct uart_buffer_s {
    volatile uint16_t in;   /* Index to the head [IN] index in the buffer */
    volatile uint16_t out;   /* Index to the tail [OUT] index in the buffer */
    uint16_t          size;
    uint16_t          capacity;
    unsigned char     *buffer;
};

struct uart_dmaxfer_s {
    unsigned char  *buffer;
    bool           halflag;
    uint16_t       capacity;
};

struct uart_dev_s;
struct uart_ops_s
{
    int (*setup)(struct uart_dev_s *dev);

    bool (*txready)(struct uart_dev_s *dev);

    /* Return true if the receive data is available */

    bool (*rxavailable)(struct uart_dev_s *dev);

    /* Start transfer bytes from the TX circular buffer using DMA */

    int (*dmasend)(struct uart_dev_s *dev, const uint8_t *p, uint16_t len);

    /* This method will send one byte on the UART */

    int (*send)(struct uart_dev_s *dev, const uint8_t *p, uint16_t len);

    int (*readbuf)(struct uart_dev_s *dev, uint8_t *p, uint16_t len);

    void (*rxclear)(struct uart_dev_s *dev);
};

struct uart_dev_s {

    /* Config data */
    int baudrate;
    unsigned char wordlen;
    unsigned char stopbitlen;
    char parity;

    /* I/O buffers */
    struct uart_buffer_s xmit;    /* Describes transmit buffer */
    struct uart_buffer_s recv;    /* Describes receive buffer */

    /* DMA transfers */
    struct uart_dmaxfer_s dmatx;  /* Describes transmit DMA transfer */
    struct uart_dmaxfer_s dmarx;  /* Describes receive DMA transfer */

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
    SemaphoreHandle_t  exclsem;     /* Prevent devices from being occupied by multiple threads */
    /**
     * Asynch Bus:
     * serial poll send: xmitsem can protect poll send action
     * serial dma send: xmitsem can protect in next xmit, 
     *                  txisr will give sem to wakeup
     * 
     * serial recv: use dma isr buffer, 
     *              recvsem protect dma isr buffer read action
     */
    SemaphoreHandle_t  xmitsem;
    SemaphoreHandle_t  recvsem;
#else
    volatile uint32_t flag_excl;

    volatile uint32_t flag_tx;
#endif

    /* Driver interface */
    const struct uart_ops_s *ops;  /* Arch-specific operations */
    void                    *priv; /* Used by the arch-specific logic */
};

typedef struct uart_dev_s uart_dev_t;

#if defined(__cplusplus)
extern "C"{
#endif

int                  serial_register(struct uart_dev_s *dev, int bus);
struct uart_dev_s*   serial_bus_get(int bus);
int                  serial_bus_initialize(int bus);

int  serial_dev_lock(struct uart_dev_s *dev);
int  serial_dev_unlock(struct uart_dev_s *dev);

int  serial_tx_wait(struct uart_dev_s *dev);

/** serial tx sem/flag post in irq(dma/txirq) */
void serial_tx_post(struct uart_dev_s *dev);

uint16_t serial_buf_write(struct uart_buffer_s *obj, const uint8_t *p, uint16_t len);
uint16_t serial_buf_read(struct uart_buffer_s *obj, uint8_t *p, uint16_t len);
void     serial_buf_clear(struct uart_buffer_s *obj);

#if defined(__cplusplus)
}
#endif

#endif  // end with macro CONFIG_GPDRIVE_SERIAL

#endif
