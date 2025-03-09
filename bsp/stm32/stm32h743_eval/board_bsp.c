#include "board_config.h"
#include <drv_uart.h>

/* COM1 */
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 512,
            .buffer = com1_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com1_txbuff,
        },
        .dmarx = {
            .capacity = 256,
            .buffer = com1_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com1_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com1_dev,
    },
    .id = 1,
    .pin_tx = 1,
    .pin_rx = 1,
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};


struct gpio_pin_t led;

void board_bsp_init()
{
    com1_dev.dev.ops->setup(&com1_dev.dev);
    led = low_gpio_setup(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, IOMODE_OUTPP, IO_SPEEDHIGH, IO_NOPULL, 0, NULL, 0);

// //     board_mtd_init();
// // #ifdef BOARD_MTD_RW_TEST
// //     board_mtd_rw_test();
// // #endif

//     board_mmcsd_init();
// #ifdef BOARD_MMCSD_RW_TEST
//     // board_mmcsd_rw_test();
// #endif

// #ifdef BSP_MODULE_USB_CHERRY
//     cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
// #endif
}

void board_blue_led_toggle()
{
    int val = low_gpio_read(&led);
    low_gpio_write(&led, !val);
}

uint8_t buff_debug[256];
void board_debug()
{
    int size = com1_dev.dev.ops->readbuf(&com1_dev.dev, &buff_debug[0], 256);
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            printf("%c", buff_debug[i]);
        }
        printf("\r\n");
    }
    board_blue_led_toggle();
}

#ifdef BSP_COM_PRINTF

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
        com1_dev.dev.ops->send(&com1_dev.dev, ptr, len);
        //drv_uart_send(&com1, ptr, len, RWPOLL);
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
    // const int stdin_fileno = 0;
    // const int stdout_fileno = 1;
    // const int stderr_fileno = 2;
    // devbuf_t buf = drv_uart_devbuf(&com1);
    // size_t rcv_size = devbuf_size(&buf);
    // size_t sld_size = (len >= rcv_size) ? rcv_size: len;
    // size_t ret_size = 0;
    // if (file == stdin_fileno) {
    //     ret_size = devbuf_read(&com1.rx_buf, ptr, sld_size);
    // }
    // return ret_size;
}
#endif


