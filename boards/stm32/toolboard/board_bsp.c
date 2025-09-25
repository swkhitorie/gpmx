#include <board_config.h>
#include <drv_uart.h>
#include <device/dnode.h>

/* COM2 */
uint8_t com2_dma_rxbuff[256];
uint8_t com2_dma_txbuff[256];
uint8_t com2_txbuff[512];
uint8_t com2_rxbuff[512];
struct up_uart_dev_s com2_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 512,
            .buffer = com2_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com2_txbuff,
        },
        .dmarx = {
            .capacity = 256,
            .buffer = com2_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com2_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com2_dev,
    },
    .id = 2,
    .txpin = { .port = GPIOA, .pin = 2, },
    .rxpin = { .port = GPIOA, .pin = 3, },
    .txdma_cfg = {
        .instance = DMA1_Channel7,
        .dma_rcc = RCC_AHBENR_DMA1EN,
        .dma_irq = DMA1_Channel7_IRQn,
        .priority = 2,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA1_Channel6,
        .dma_rcc = RCC_AHBENR_DMA1EN,
        .dma_irq = DMA1_Channel6_IRQn,
        .priority = 1,
        .enable = true,
    },
    .priority = 1,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(100);
    LOW_INITPIN(GPIOB, 9, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    serial_register(&com2_dev.dev, 2);
    serial_bus_initialize(2);

    dstdout = serial_bus_get(2);
    dstdin = serial_bus_get(2);
}

void board_debug()
{
    int val = LOW_IOGET(GPIOB, 9);
    LOW_IOSET(GPIOB, 9, !val);
}

#ifdef CONFIG_BOARD_COM_STDINOUT
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
int _write(int file, const char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
#ifdef CONFIG_BOARD_COM_STDOUT_DMA
        SERIAL_DMASEND(dstdout, ptr, len);
#else
        SERIAL_SEND(dstdout, ptr, len);
#endif
    }
    return len;
}
// nonblock
int _read(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    int rsize = 0;
    if (file == stdin_fileno) {
        rsize = SERIAL_RDBUF(dstdin, ptr, len);
    }
    return rsize;
}
size_t fwrite(const void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _write(stream->_file, ptr, size*n_items);
}
size_t fread(void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _read(stream->_file, ptr, size*n_items);
}
#endif

