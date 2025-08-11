#include <board_config.h>
#include <drv_uart.h>
#include <device/dnode.h>
#include <device/serial.h>

#define MSG_SENDER_DMA_TX_SIZE      (128)
#define MSG_SENDER_TX_SIZE          (128)
#define MSG_SENDER_DMA_RX_SIZE      (1024*4)
#define MSG_SENDER_RX_SIZE          (1024*2)

#define MSG_RECEIVER_DMA_TX_SIZE      (1024)
#define MSG_RECEIVER_TX_SIZE          (1024)
#define MSG_RECEIVER_DMA_RX_SIZE      (128)
#define MSG_RECEIVER_RX_SIZE          (128)

int board_role;

/* log com buffer config */
uint8_t log_dma_rxbuff[128];
uint8_t log_dma_txbuff[256];
uint8_t log_txbuff[256];
uint8_t log_rxbuff[128];

/* lora msg com buffer config, ROLE: Sender */
uint8_t msg_dma_txbuff_sender[MSG_SENDER_DMA_TX_SIZE];
uint8_t msg_txbuff_sender[MSG_SENDER_TX_SIZE];
uint8_t msg_dma_rxbuff_sender[MSG_SENDER_DMA_RX_SIZE];
uint8_t msg_rxbuff_sender[MSG_SENDER_RX_SIZE];

/* lora msg com buffer config, ROLE: Receiver */
uint8_t msg_dma_txbuff_receiver[MSG_RECEIVER_DMA_TX_SIZE];
uint8_t msg_txbuff_receiver[MSG_RECEIVER_TX_SIZE];
uint8_t msg_dma_rxbuff_receiver[MSG_RECEIVER_DMA_RX_SIZE];
uint8_t msg_rxbuff_receiver[MSG_RECEIVER_RX_SIZE];

struct up_uart_dev_s com2_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 128,
            .buffer = log_rxbuff,
        },
        .xmit = {
            .capacity = 256,
            .buffer = log_txbuff,
        },
        .dmarx = {
            .capacity = 128,
            .buffer = log_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = log_dma_txbuff,
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

uart_dev_t *_tty_log_out;
uart_dev_t *_tty_log_in;

uart_dev_t *_tty_msg_out;
uart_dev_t *_tty_msg_in;

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(200);

    /* LED Array */
    LOW_INITPIN(GPIOB, 15, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOB, 9, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOB, 11, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    board_role = RADIO_BOARD_ROLE;

    serial_register(&com1_dev.dev, 1);
    serial_register(&com2_dev.dev, 2);

    serial_bus_initialize(1);
    serial_bus_initialize(2);

    switch (board_role) {
    case RADIO_BOARD_TRANSMITTER: {
        com1_dev.dev.recv.capacity = MSG_SENDER_RX_SIZE;
        com1_dev.dev.recv.buffer = msg_rxbuff_sender;
        com1_dev.dev.xmit.capacity = MSG_SENDER_TX_SIZE;
        com1_dev.dev.xmit.buffer = msg_txbuff_sender;

        com1_dev.dev.dmarx.capacity = MSG_SENDER_DMA_RX_SIZE;
        com1_dev.dev.dmarx.buffer = msg_dma_rxbuff_sender;
        com1_dev.dev.dmatx.capacity = MSG_SENDER_DMA_TX_SIZE;
        com1_dev.dev.dmatx.buffer = msg_dma_txbuff_sender;
        }
        break;
    case RADIO_BOARD_RECEIVER: {
        com1_dev.dev.recv.capacity = MSG_RECEIVER_RX_SIZE;
        com1_dev.dev.recv.buffer = msg_rxbuff_receiver;
        com1_dev.dev.xmit.capacity = MSG_RECEIVER_TX_SIZE;
        com1_dev.dev.xmit.buffer = msg_txbuff_receiver;

        com1_dev.dev.dmarx.capacity = MSG_RECEIVER_DMA_RX_SIZE;
        com1_dev.dev.dmarx.buffer = msg_dma_rxbuff_receiver;
        com1_dev.dev.dmatx.capacity = MSG_RECEIVER_DMA_TX_SIZE;
        com1_dev.dev.dmatx.buffer = msg_dma_txbuff_receiver;
        }
        break;
    default:break;
    }

    _tty_log_out = serial_bus_get(2);
    _tty_log_in = serial_bus_get(2);
    _tty_msg_out = serial_bus_get(1);
    _tty_msg_in = serial_bus_get(1);

    SERIAL_RXCLEAR(_tty_msg_in);
    SERIAL_RXCLEAR(_tty_log_in);
}

void board_debug()
{
    static int timer_led = 0;
    if (++timer_led > 100) {
        timer_led = 1;
        board_led_toggle(1);
    }
}

void board_led_toggle(int i)
{
    int val;
    switch (i) {
    case 0:
        val = LOW_IOGET(GPIOB, 11);
        LOW_IOSET(GPIOB, 11, !val);
        break;
    case 1:
        val = LOW_IOGET(GPIOB, 9);
        LOW_IOSET(GPIOB, 9, !val);
        break;
    case 2:
        val = LOW_IOGET(GPIOB, 15);
        LOW_IOSET(GPIOB, 15, !val);
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
#ifdef CONFIG_BOARD_COM_STDOUT_DMA
        SERIAL_DMASEND(_tty_log_out, ptr, len);
#else
        SERIAL_SEND(_tty_log_out, ptr, len);
#endif
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
        rsize = SERIAL_RDBUF(_tty_log_in, ptr, len);
    }
    return rsize;
}
#endif

void board_get_uid(uint32_t *p)
{
    p[0] = *(volatile uint32_t*)(0x1FFF7590);
    p[1] = *(volatile uint32_t*)(0x1FFF7594);
    p[2] = *(volatile uint32_t*)(0x1FFF7598);
}

uint32_t board_elapsed_tick(const uint32_t tick)
{
    uint32_t now = HAL_GetTick();
    if (tick > now) {
        return 0;
    }
    return now - tick;
}

int board_get_role() { return board_role; }
