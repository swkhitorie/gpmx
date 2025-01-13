#include "board_config.h"
#include <stdio.h>
#include <string.h>

struct drv_uart_t com2;
struct drv_uart_attr_t com2_attr;
struct drv_uart_dma_attr_t com2_rxdma_attr;
struct drv_uart_dma_attr_t com2_txdma_attr;
uint8_t com2_dma_rxbuff[256];
uint8_t com2_dma_txbuff[256];
uint8_t com2_txbuff_mem[512];
uint8_t com2_rxbuff_mem[512];

char buffer_tx_dma_test[128];

void board_bsp_init()
{
    drv_uart_dma_attr_init(&com2_rxdma_attr, &com2_dma_rxbuff[0], 256, 1);
    drv_uart_dma_attr_init(&com2_txdma_attr, &com2_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com2_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 0x01);
    drv_uart_buff_init(&com2, &com2_txbuff_mem[0], 512, 
                       &com2_rxbuff_mem[0], 512);
    drv_uart_init(2, &com2, &com2_attr, &com2_txdma_attr, &com2_rxdma_attr);
}

void board_debug()
{
    // sprintf(&buffer_tx_dma_test[0], "[%d] gd32 test txdma \r\n", board_tickget());
    // drv_uart_send(&com2, (const uint8_t *)&buffer_tx_dma_test[0], strlen(buffer_tx_dma_test), 1);
    printf("com1 rsize: %d \r\n", devbuf_size(&com2.rx_buf));
}

int fputc(int ch, FILE *f)
{
    uint8_t c = ch;
    drv_uart_send(&com2, (const uint8_t *)&c, 1, 0);
    return ch;
}
