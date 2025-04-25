#include <board_config.h>
#include <drv_uart.h>
#include <dev/dnode.h>
#include <dev/serial.h>

#include "ringbuffer.h"

#include "app_subghz.h"
#include "radio.h"

#define DEBUG_COM                  (2)
#define TRANS_COM                  (1)
#define TRANS_BAUDRATE             (460800)

#if RADIO_ROLE == RADIO_TRANSMITTER
#define COM_RXSIZE     (1024*2)
#define COM_RXDMASIZE  (1024*4)
#define COM_TXSIZE     (128)
#define COM_TXDMASIZE  (128)

#define RADIO_RXSIZE   (128)
#elif RADIO_ROLE == RADIO_RECEIVER
#define COM_RXSIZE     (128)
#define COM_RXDMASIZE  (128)
#define COM_TXSIZE     (1024*2)
#define COM_TXDMASIZE  (1024*2)

#define RADIO_RXSIZE   (1024)
#endif

/* debug com */
uint8_t debug_dma_rxbuff[64];
uint8_t debug_dma_txbuff[64];
uint8_t debug_txbuff[128];
uint8_t debug_rxbuff[128];
struct up_uart_dev_s debug_dev = {
    .dev = {
        .baudrate = 460800,
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
        .priv      = &debug_dev,
    },
    .id = DEBUG_COM,
    .pin_tx = 0, //PA9/PA2 
    .pin_rx = 0, //PA10/PA3
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/* COM2 */
uint8_t trans_dma_rxbuff[COM_RXDMASIZE];
uint8_t trans_dma_txbuff[COM_TXDMASIZE];
uint8_t trans_txbuff[COM_TXSIZE];
uint8_t trans_rxbuff[COM_RXSIZE];
struct up_uart_dev_s trans_dev = {
    .dev = {
        .baudrate = TRANS_BAUDRATE,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = COM_RXSIZE,
            .buffer = trans_rxbuff,
        },
        .xmit = {
            .capacity = COM_TXSIZE,
            .buffer = trans_txbuff,
        },
        .dmarx = {
            .capacity = COM_RXDMASIZE,
            .buffer = trans_dma_rxbuff,
        },
        .dmatx = {
            .capacity = COM_TXDMASIZE,
            .buffer = trans_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &trans_dev,
    },
    .id = TRANS_COM,
    .pin_tx = 0, //PA9/PA2 
    .pin_rx = 0, //PA10/PA3 
    .priority = 1,
    .priority_dmarx = 0,
    .priority_dmatx = 1,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

uart_dev_t *dtransout;
uart_dev_t *dtransin;

uint8_t radio_rxmem[RADIO_RXSIZE];
ringbuffer_t radio_rxbuf = {
    .buf = &radio_rxmem[0],
    .capacity = RADIO_RXSIZE,
    .in = 0,
    .out = 0,
    .size = 0,
};

char radio_sync_buf[16];
char send_head[] = "SEND";
char rcvs_head[] = "RECV";
uint8_t sync_step = 0;
uint8_t tx_sync = 0;

bool board_radio_sync_already()
{
    return sync_step == 1;
}

void board_radio_txflag_clr()
{
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

    #if RADIO_ROLE == RADIO_TRANSMITTER
        switch (sync_step) {
        case 0:
            if (sz > 0 && !strcmp(radio_sync_buf, "RECV")) {
                sync_step++;
            } else {
                Radio.Send(send_head, strlen(send_head));
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
            if (sz > 0 && !strcmp(radio_sync_buf, "SEND")) {
                Radio.Send(rcvs_head, strlen(rcvs_head));
                HAL_Delay(100);
                sync_step++;
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
    low_gpio_setup(GPIOB, 15, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
    /* LED2 */
    low_gpio_setup(GPIOB, 9, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);
    /* LED3 */
    low_gpio_setup(GPIOB, 11, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH, 0, NULL, 0);

    dregister("/com_debug", &debug_dev.dev);
    dregister("/com_trans", &trans_dev.dev);

    debug_dev.dev.ops->setup(&debug_dev.dev);
    trans_dev.dev.ops->setup(&trans_dev.dev);

    dstdout = dbind("/com_debug");
    dstdin = dbind("/com_debug");
    dtransout = dbind("/com_trans");
    dtransin = dbind("/com_trans");

    app_subghz_init();

    bool ret = false;
    while (!ret) {
        ret = board_radio_sync();
        HAL_Delay(500);
    }
    printf("sync completed \r\n");
}

static int buff_trans[300];
static int buff_reads[256];
static int timer_led = 0;
void board_debug()
{

#if RADIO_ROLE == RADIO_TRANSMITTER
    if (tx_sync == 0 && READ_BIT(SUBGHZSPI->SR, SPI_SR_TXE) == (SPI_SR_TXE)) {
        int sz = SERIAL_RDBUF(dtransin, buff_trans, MaxUserPayloadSize);
        if (sz > 0) {
            tx_sync = 1;
            Radio.Send(buff_trans, sz);
            if (sz >= MaxUserPayloadSize) {
                HAL_Delay(40);
            }
            SERIAL_SEND(dtransout, buff_trans, sz);
        }
    }
#endif

#if RADIO_ROLE == RADIO_RECEIVER
    int rsize = rb_size(&radio_rxbuf);
    if (rsize > 0) {
        int sz = rb_read(&radio_rxbuf, &buff_reads[0], 256);
        SERIAL_SEND(dtransout, buff_reads, sz);
    }
#endif

    if (++timer_led > 100) {
        timer_led = 1;
        #if RADIO_ROLE == RADIO_RECEIVER
        board_led_toggle(2);
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
