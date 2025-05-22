#include <board_config.h>
#include <drv_uart.h>
#include <device/dnode.h>
#include <device/serial.h>

#include "app_subghz.h"

#define TRANS_SENDER_DMA_TX_SIZE      (128)
#define TRANS_SENDER_TX_SIZE          (128)
#define TRANS_SENDER_DMA_RX_SIZE      (1024*4)
#define TRANS_SENDER_RX_SIZE          (1024*2)

#define TRANS_RECVER_DMA_TX_SIZE      (1024)
#define TRANS_RECVER_TX_SIZE          (1024)
#define TRANS_RECVER_DMA_RX_SIZE      (128)
#define TRANS_RECVER_RX_SIZE          (128)

char role_str[2][16] = {
    {"RADIO_SENDER"},
    {"RADIO_RECEIVER"},
};

int board_role = RADIO_SENDER;
int role_led_id = 0;

/* debug com */
uint8_t debug_dma_rxbuff[64];
uint8_t debug_dma_txbuff[64];
uint8_t debug_txbuff[128];
uint8_t debug_rxbuff[128];

// ROLE: Sender
uint8_t trans_dma_txbuff_sender[TRANS_SENDER_DMA_TX_SIZE];
uint8_t trans_txbuff_sender[TRANS_SENDER_TX_SIZE];
uint8_t trans_dma_rxbuff_sender[TRANS_SENDER_DMA_RX_SIZE];
uint8_t trans_rxbuff_sender[TRANS_SENDER_RX_SIZE];

// ROLE: Receiver
uint8_t trans_dma_txbuff_receiver[TRANS_RECVER_DMA_TX_SIZE];
uint8_t trans_txbuff_receiver[TRANS_RECVER_TX_SIZE];
uint8_t trans_dma_rxbuff_receiver[TRANS_RECVER_DMA_RX_SIZE];
uint8_t trans_rxbuff_receiver[TRANS_RECVER_RX_SIZE];

struct up_uart_dev_s com2_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 128,
            .buffer = debug_rxbuff,
        },
        .xmit = {
            .capacity = 128,
            .buffer = debug_txbuff,
        },
        .dmarx = {
            .capacity = 64,
            .buffer = debug_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 64,
            .buffer = debug_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com2_dev,
    },
    .id = 2,     //USART2
    .pin_tx = 0, //USART2 PA2
    .pin_rx = 0, //USART2 PA3
    .priority = 4,
    .priority_dmarx = 5,
    .priority_dmatx = 6,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 0,
            .buffer = NULL,
        },
        .xmit = {
            .capacity = 0,
            .buffer = NULL,
        },
        .dmarx = {
            .capacity = 0,
            .buffer = NULL,
        },
        .dmatx = {
            .capacity = 0,
            .buffer = NULL,
        },
        .ops       = &g_uart_ops,
        .priv      = &com1_dev,
    },
    .id = 1,     //USART1
    .pin_tx = 0, //USART1 PA9
    .pin_rx = 0, //USART1 PA10
    .priority = 1,
    .priority_dmarx = 1,
    .priority_dmatx = 2,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

uart_dev_t *dtransout;
uart_dev_t *dtransin;

int board_get_role() { return board_role; }

uart_dev_t *board_get_stream_transout() { return dtransout; }

uart_dev_t *board_get_stream_transin() { return dtransin; }

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(200);

    /* LED1 */
    low_gpio_setup(GPIOB, 15, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
    /* LED2 */
    low_gpio_setup(GPIOB, 9, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
    /* LED3 */
    low_gpio_setup(GPIOB, 11, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);

    dregister("/com1", &com1_dev.dev);
    dregister("/com2", &com2_dev.dev);
    com2_dev.dev.ops->setup(&com2_dev.dev);
    com1_dev.dev.ops->setup(&com1_dev.dev);

    switch (board_role) {
    case RADIO_SENDER:
        {
            com1_dev.dev.recv.capacity = TRANS_SENDER_RX_SIZE;
            com1_dev.dev.recv.buffer = trans_rxbuff_sender;
            com1_dev.dev.xmit.capacity = TRANS_SENDER_TX_SIZE;
            com1_dev.dev.xmit.buffer = trans_txbuff_sender;

            com1_dev.dev.dmarx.capacity = TRANS_SENDER_DMA_RX_SIZE;
            com1_dev.dev.dmarx.buffer = trans_dma_rxbuff_sender;
            com1_dev.dev.dmatx.capacity = TRANS_SENDER_DMA_TX_SIZE;
            com1_dev.dev.dmatx.buffer = trans_dma_txbuff_sender;
        }
        break;
    case RADIO_RECEIVER:
        {
            com1_dev.dev.recv.capacity = TRANS_RECVER_RX_SIZE;
            com1_dev.dev.recv.buffer = trans_rxbuff_receiver;
            com1_dev.dev.xmit.capacity = TRANS_RECVER_TX_SIZE;
            com1_dev.dev.xmit.buffer = trans_txbuff_receiver;

            com1_dev.dev.dmarx.capacity = TRANS_RECVER_DMA_RX_SIZE;
            com1_dev.dev.dmarx.buffer = trans_dma_rxbuff_receiver;
            com1_dev.dev.dmatx.capacity = TRANS_RECVER_DMA_TX_SIZE;
            com1_dev.dev.dmatx.buffer = trans_dma_txbuff_receiver;
        }
        break;
    default:break;
    }

    dstdout = dbind("/com2");
    dstdin = dbind("/com2");
    dtransout = dbind("/com1");
    dtransin = dbind("/com1");

    switch (board_role) {
    case RADIO_SENDER:
        role_led_id = 0;
        break;
    case RADIO_RECEIVER:
        role_led_id = 2;
        break;
    default:break;
    }

    app_subghz_init(board_role);

    SERIAL_RXCLEAR(dstdin);
    SERIAL_RXCLEAR(dtransin);
    printf("[bsp] init completed \r\n");
}

uint8_t buffa[128];
uint8_t buffb[128];

static int timer_led = 0;
void board_debug()
{
    static int timer_led = 0;
    if (++timer_led > 100) {
        timer_led = 1;
        board_led_toggle(role_led_id);
    }

    // int r1sz = SERIAL_RDBUF(dtransin, buffa, 128);
    // if (r1sz > 0) {
    //     SERIAL_SEND(dtransin, buffa, r1sz);
    // }

    // int r2sz = SERIAL_RDBUF(dstdin, buffb, 128);
    // if (r2sz > 0) {
    //     SERIAL_SEND(dstdin, buffb, r2sz);
    // }
}

void board_led_toggle(int i)
{
    int val;
    switch (i) {
    case 0:
        val = HAL_GPIO_ReadPin(GPIOB, 1<<11);
        HAL_GPIO_WritePin(GPIOB, 1<<11, !val);
        break;
    case 1:
        val = HAL_GPIO_ReadPin(GPIOB, 1<<9);
        HAL_GPIO_WritePin(GPIOB, 1<<9, !val);
        break;
    case 2:
        val = HAL_GPIO_ReadPin(GPIOB, 1<<15);
        HAL_GPIO_WritePin(GPIOB, 1<<15, !val);
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
