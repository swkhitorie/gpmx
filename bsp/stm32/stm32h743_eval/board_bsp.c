#include "board_config.h"
#include <drv_uart.h>
#include <dev/dnode.h>
#include <dev/serial.h>

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

uart_dev_t *dstdout = &com1_dev.dev;
uart_dev_t *dstdin = &com1_dev.dev;

void board_bsp_init()
{
	BOARD_INIT_IOPORT(0, GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

    dregister("/com1", &com1_dev.dev);

    com1_dev.dev.ops->setup(&com1_dev.dev);

    dstdout = dbind("/com1");
    dstdin = dbind("/com1");

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
    int val = BOARD_IO_GET(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN);
    BOARD_IO_SET(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, !val);
}

uint8_t buff_debug[256];
void board_debug()
{
    int size = SERIAL_RDBUF(dstdin, buff_debug, 256);
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
