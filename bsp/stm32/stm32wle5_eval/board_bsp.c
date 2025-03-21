#include <board_config.h>
#include <drv_uart.h>
#include <dev/dnode.h>
#include <dev/serial.h>

#include "ringbuffer.h"

#include "app_subghz.h"
#include "radio.h"

/* COM1 */
uint8_t com1_dma_rxbuff[128];
uint8_t com1_dma_txbuff[128];
uint8_t com1_txbuff[256];
uint8_t com1_rxbuff[256];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 256,
            .buffer = com1_rxbuff,
        },
        .xmit = {
            .capacity = 256,
            .buffer = com1_txbuff,
        },
        .dmarx = {
            .capacity = 128,
            .buffer = com1_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 128,
            .buffer = com1_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com1_dev,
    },
    .id = 1, //usart1
    .pin_tx = 0, //PA9
    .pin_rx = 0, //PA10
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/* COM2 */
uint8_t com2_dma_rxbuff[1024*4];
uint8_t com2_dma_txbuff[1024];
uint8_t com2_txbuff[1024];
uint8_t com2_rxbuff[1024*4];
struct up_uart_dev_s com2_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 1024*4,
            .buffer = com2_rxbuff,
        },
        .xmit = {
            .capacity = 1024,
            .buffer = com2_txbuff,
        },
        .dmarx = {
            .capacity = 1024*4,
            .buffer = com2_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 1024,
            .buffer = com2_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com2_dev,
    },
    .id = 2, //usart2
    .pin_tx = 0, //PA2
    .pin_rx = 0, //PA3
    .priority = 1,
    .priority_dmarx = 0,
    .priority_dmatx = 1,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

uint8_t radio_rxmem[1024*2];
ringbuffer_t radio_rxbuf = {
    .buf = &radio_rxmem[0],
    .capacity = 1024*2,
    .in = 0,
    .out = 0,
    .size = 0,
};

char radio_sync_buf[16];

/**
 * radio connection establish process:
 * sender send "SEND_SYNC1"
 * receiver send "RECV_SYNC1"
 */
char sendbuff1[] = "SEND_SYNC1";
char rcvsbuff1[] = "RECV_SYNC1";
uint8_t sync_step = 0;
uint8_t tx_sync = 0;

bool board_radio_sync_already()
{
    return sync_step == 1;
}

void board_radio_txflag_clr()
{
    // printf("tx cleared \r\n");
    tx_sync = 0;
}

int board_radio_rxbuf_write(uint8_t *p, uint16_t size)
{
    return rb_write(&radio_rxbuf, p, size);
}

bool board_radio_sync()
{
    bool ret = false;

    int sz = rb_read(&radio_rxbuf, &radio_sync_buf[0], 128);
    printf("rcv: %s  %d \r\n", radio_sync_buf, sz);
    #if RADIO_ROLE == RADIO_TRANSMITTER
        switch (sync_step) {
        case 0:
            if (sz > 0 && !strcmp(radio_sync_buf, "RECV_SYNC1")) {
                sync_step++;
                printf("trans: send SEND_TACK1 \r\n");
            } else {
                Radio.Send(sendbuff1, strlen(sendbuff1));
                HAL_Delay(100);
            }
            break;
        case 1:
            ret = true;
            break;
        }
    #endif

    #if RADIO_ROLE == RADIO_RECEIVER
        switch (sync_step) {
        case 0:
            if (sz > 0 && !strcmp(radio_sync_buf, "SEND_SYNC1")) {
                Radio.Send(rcvsbuff1, strlen(rcvsbuff1));
                HAL_Delay(100);
                sync_step++;
                printf("recv: recv SEND_SYNC1  \r\n");
            }
            break;
        case 1:
            ret = true;
            Radio.Rx(100);
            break;
        }
    #endif

    return ret;
}

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(200);

    /* LED1 */
    low_gpio_setup(GPIOB, 3, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
    /* LED2 */
    low_gpio_setup(GPIOB, 4, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);

    dregister("/com1", &com1_dev.dev);
    dregister("/com2", &com2_dev.dev);

    com1_dev.dev.ops->setup(&com1_dev.dev);
    com2_dev.dev.ops->setup(&com2_dev.dev);

    dstdout = dbind("/com2");
    dstdin = dbind("/com2");

    app_subghz_init();

    bool ret = false;
    while (!ret) {
        ret = board_radio_sync();
        HAL_Delay(500);
    }
    printf("sync completed \r\n");
}

int buff_trans[1024];
int buff_reads[256];
static int timer_led = 0;
void board_debug()
{
// int sz = SERIAL_RDBUF(dstdin, buff_trans, 256);
// if (sz > 0) {
//     SERIAL_DMASEND(dstdout, buff_trans, sz);
// }

#if RADIO_ROLE == RADIO_TRANSMITTER

    if (tx_sync == 0 /*&& READ_BIT(SUBGHZSPI->SR, SPI_SR_TXE) == (SPI_SR_TXE)*/) {
        int sz = SERIAL_RDBUF(dstdin, buff_trans, MaxUserPayloadSize);
        if (sz > 0) {
            tx_sync = 1;
            Radio.Send(buff_trans, sz);
            if (sz >= MaxUserPayloadSize) {
                HAL_Delay(10);
            }
            SERIAL_SEND(dstdout, buff_trans, sz);
        }
    }
#endif

#if RADIO_ROLE == RADIO_RECEIVER
    int rsize = rb_size(&radio_rxbuf);
    if (rsize > 0) {
        int sz = rb_read(&radio_rxbuf, &buff_reads[0], 256);
        SERIAL_SEND(dstdout, buff_reads, sz);
    }
#endif

    if (++timer_led > 100) {
        timer_led = 1;
        #if RADIO_ROLE == RADIO_RECEIVER
        board_led_toggle(1);
        #endif
        #if RADIO_ROLE == RADIO_TRANSMITTER
        board_led_toggle(0);
        #endif
    }
}

void board_led_toggle(int i)
{
    int val;
    switch (i) {
    case 0:
        val = HAL_GPIO_ReadPin(GPIOB, 1<<3);
        HAL_GPIO_WritePin(GPIOB, 1<<3, !val);
        break;
    case 1:
        val = HAL_GPIO_ReadPin(GPIOB, 1<<4);
        HAL_GPIO_WritePin(GPIOB, 1<<4, !val);
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
