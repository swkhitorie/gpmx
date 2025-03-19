#include <board_config.h>
#include <drv_uart.h>
#include <dev/dnode.h>
#include <dev/serial.h>

#include "app_subghz.h"
#include "radio.h"

/* COM1 */
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
    .id = 2, //usart2
    .pin_tx = 0, //PA2
    .pin_rx = 0, //PA3
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(200);

    /* LED1 */
    low_gpio_setup(GPIOB, 3, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
    /* LED2 */
    low_gpio_setup(GPIOB, 4, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);

    dregister("/com2", &com2_dev.dev);

    com2_dev.dev.ops->setup(&com2_dev.dev);

    dstdout = dbind("/com2");
    dstdin = dbind("/com2");

    app_subghz_init();
}

void board_debug()
{
    printf("hello world stel55 \r\n");
    // Radio.Send(test_rbf, strlen(test_rbf));
    board_led_toggle(0);
}

void board_led_toggle(int i)
{
    int val;
    switch (i) {
    case 0:
        val = HAL_GPIO_ReadPin(GPIOB, 1<<3);
        HAL_GPIO_WritePin(GPIOB, 1<<11, !val);
        break;
    case 1:
        val = HAL_GPIO_ReadPin(GPIOB, 1<<4);
        HAL_GPIO_WritePin(GPIOB, 1<<9, !val);
        break;
    }
}

#ifdef CONFIG_BOARD_COM_STDINOUT
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
size_t fwrite(const void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _write(stream->_file, ptr, size*n_items);
}
int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
        SERIAL_SEND(dstdout, ptr, len);
    }
    return len;
}
size_t fread(void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _read(stream->_file, ptr, size*n_items);
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
#endif
