#include "board_config.h"
#include <drv_uart.h>


struct drv_uart_t com1;
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];

struct drv_pin_t led;
struct drv_pin_irq_t io1_irq;
struct drv_pin_t io1;
void io1_trigger() { printf("io1_trigger\r\n"); }

void board_bsp_init()
{
    struct drv_uart_attr_t com1_attr;
    struct drv_uart_dma_attr_t com1_rxdma_attr;
    struct drv_uart_dma_attr_t com1_txdma_attr;
    drv_uart_dma_attr_init(&com1_rxdma_attr, &com1_dma_rxbuff[0], 256, 2);
    drv_uart_dma_attr_init(&com1_txdma_attr, &com1_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com1_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 1);
    drv_uart_buff_init(&com1, &com1_txbuff[0], 512, &com1_rxbuff[0], 512);
    drv_uart_init(1, &com1, 1, 1, &com1_attr, &com1_txdma_attr, &com1_rxdma_attr);

    led = drv_gpio_init(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, IOMODE_OUTPP, 
                IO_SPEEDHIGH, IO_NOPULL, 0, NULL);
    drv_gpio_irq_init(&io1_irq, 6, io1_trigger);
    io1 = drv_gpio_init(GPIOB, 8, IOMODE_IT_RISING, IO_SPEEDHIGH, IO_PULLDOWN, 0, &io1_irq);

//     board_mtd_init();
// #ifdef BOARD_MTD_RW_TEST
//     board_mtd_rw_test();
// #endif

    board_mmcsd_init();
#ifdef BOARD_MMCSD_RW_TEST
    board_mmcsd_rw_test();
#endif

#ifdef BSP_MODULE_USB_CHERRY
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
}

void board_blue_led_toggle()
{
    int val = drv_gpio_read(&led);
    drv_gpio_write(&led, !val);
}

uint8_t buff_debug[256];
void board_debug()
{
    devbuf_t buf = drv_uart_devbuf(&com1);
    int size = devbuf_size(&buf);
    if (size > 0) {
        printf("[%d,%d] ", HAL_GetTick(),size);
        devbuf_read(&com1.rx_buf, &buff_debug[0], size);
        for (int i = 0; i < size; i++) {
            printf("%d ", buff_debug[i]);
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
        drv_uart_send(&com1, ptr, len, RWPOLL);
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
    devbuf_t buf = drv_uart_devbuf(&com1);
    size_t rcv_size = devbuf_size(&buf);
    size_t sld_size = (len >= rcv_size) ? rcv_size: len;
    size_t ret_size = 0;
    if (file == stdin_fileno) {
        ret_size = devbuf_read(&com1.rx_buf, ptr, sld_size);
    }
    return ret_size;
}
#endif


