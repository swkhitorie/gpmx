#include "board_config.h"
#include <drv_uart.h>

struct drv_uart_t com1;
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];

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
}

void board_debug()
{
    printf("rx :%d \r\n", devbuf_size(&com1.rx_buf));
}



#ifdef BSP_COM_PRINTF
#include <stdlib.h>
#include <stdio.h>
FILE __stdin, __stdout, __stderr;
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

#endif
