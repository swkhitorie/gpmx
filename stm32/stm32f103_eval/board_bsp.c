#include <board_config.h>
#include <drv_uart.h>

#ifdef BSP_COM_PRINTF
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
#endif

struct drv_uart_t com2;
uint8_t com2_dma_rxbuff[256];
uint8_t com2_dma_txbuff[256];
uint8_t com2_txbuff[512];
uint8_t com2_rxbuff[512];

struct drv_pin_t led;

void board_bsp_init()
{
    struct drv_uart_attr_t com2_attr;
    struct drv_uart_dma_attr_t com2_rxdma_attr;
    struct drv_uart_dma_attr_t com2_txdma_attr;
    drv_uart_dma_attr_init(&com2_rxdma_attr, &com2_dma_rxbuff[0], 256, 2);
    drv_uart_dma_attr_init(&com2_txdma_attr, &com2_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com2_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 1);
    drv_uart_buff_init(&com2, &com2_txbuff[0], 512, &com2_rxbuff[0], 512);
    drv_uart_init(2, &com2, 0, 0, &com2_attr, &com2_txdma_attr, &com2_rxdma_attr);

    led = drv_gpio_init(GPIOB, 9, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, NULL);
}

uint8_t buff_debug[256];
void board_debug()
{
    devbuf_t buf = drv_uart_devbuf(&com2);
    int size = devbuf_size(&buf);
    if (size > 0) {
        fprintf(stdout, "[%d,%d] ", HAL_GetTick(),size);
        devbuf_read(&com2.rx_buf, &buff_debug[0], size);
        for (int i = 0; i < size; i++) {
            fprintf(stdout, "%d ", buff_debug[i]);
        }
        fprintf(stdout, "\r\n");
    }
    int val = drv_gpio_read(&led);
    drv_gpio_write(&led, !val);
}

#ifdef BSP_COM_PRINTF
int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
        drv_uart_send(&com2, ptr, len, RWPOLL);
    }
    return len;
}
// nonblock
int _read(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    devbuf_t buf = drv_uart_devbuf(&com2);
    size_t rcv_size = devbuf_size(&buf);
    size_t sld_size = (len >= rcv_size) ? rcv_size: len;
    size_t ret_size = 0;
    if (file == stdin_fileno) {
        ret_size = devbuf_read(&com2.rx_buf, ptr, sld_size);
    }
    return ret_size;
}
#endif
