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
#define MSG_RECEIVER_DMA_RX_SIZE      (3000)
#define MSG_RECEIVER_RX_SIZE          (3000)

int board_role;
/* log com buffer config */
uint8_t log_dma_rxbuff[128] = {0};
uint8_t log_dma_txbuff[256] = {0};
uint8_t log_txbuff[256] = {0};
uint8_t log_rxbuff[128] = {0};

/* lora msg com buffer config, ROLE: Sender */
uint8_t msg_dma_txbuff_sender[MSG_SENDER_DMA_TX_SIZE] = {0};
uint8_t msg_txbuff_sender[MSG_SENDER_TX_SIZE] = {0};
uint8_t msg_dma_rxbuff_sender[MSG_SENDER_DMA_RX_SIZE] = {0};
uint8_t msg_rxbuff_sender[MSG_SENDER_RX_SIZE] = {0};

/* lora msg com buffer config, ROLE: Receiver */
uint8_t msg_dma_txbuff_receiver[MSG_RECEIVER_DMA_TX_SIZE] = {0};
uint8_t msg_txbuff_receiver[MSG_RECEIVER_TX_SIZE] = {0};
uint8_t msg_dma_rxbuff_receiver[MSG_RECEIVER_DMA_RX_SIZE] = {0};
uint8_t msg_rxbuff_receiver[MSG_RECEIVER_RX_SIZE] = {0};

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
    .txpin = { .port = GPIOA, .pin = 2,  .alternate = GPIO_AF7_USART2,},
    .rxpin = { .port = GPIOA, .pin = 3,  .alternate = GPIO_AF7_USART2,},
    .txdma_cfg = {
        .instance = DMA1_Channel2,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Channel2_IRQn,
        .request = DMA_REQUEST_USART2_TX,
        .priority = 6,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA2_Channel2,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Channel2_IRQn,
        .request = DMA_REQUEST_USART2_RX,
        .priority = 5,
        .enable = true,
    },
    .priority = 4,
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
    .txpin = { .port = GPIOA, .pin = 9,   .alternate = GPIO_AF7_USART1,},
    .rxpin = { .port = GPIOA, .pin = 10,  .alternate = GPIO_AF7_USART1,},
    .txdma_cfg = {
        .instance = DMA1_Channel1,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Channel1_IRQn,
        .request = DMA_REQUEST_USART1_TX,
        .priority = 2,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA2_Channel1,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Channel1_IRQn,
        .request = DMA_REQUEST_USART1_RX,
        .priority = 1,
        .enable = true,
    },
    .priority = 1,
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
    LOW_INITPIN(GPIOB, 3, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOB, 4, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    board_role = RADIO_BOARD_ROLE;
    serial_register(&com1_dev.dev, 1);
    serial_register(&com2_dev.dev, 2);

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

    serial_bus_initialize(1);
    serial_bus_initialize(2);

    _tty_log_out = serial_bus_get(2);
    _tty_log_in = serial_bus_get(2);
    _tty_msg_out = serial_bus_get(1);
    _tty_msg_in = serial_bus_get(1);

    SERIAL_RXCLEAR(_tty_msg_in);
    SERIAL_RXCLEAR(_tty_log_in);

    board_rng_init();
    board_crc_init();
    board_subghz_init();

}

void board_bsp_deinit()
{
    __set_PRIMASK(1);

    board_rng_deinit();
    board_crc_deinit();
    board_subghz_deinit();

    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();

    __HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_USART2_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    HAL_DMA_DeInit(com1_dev.com.hdmatx);
    HAL_DMA_DeInit(com1_dev.com.hdmarx);

    HAL_DMA_DeInit(com2_dev.com.hdmatx);
    HAL_DMA_DeInit(com2_dev.com.hdmarx);

    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_NVIC_DisableIRQ(USART2_IRQn);

    HAL_DeInit();

    __set_PRIMASK(0); 
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
        val = LOW_IOGET(GPIOB, 4);
        LOW_IOSET(GPIOB, 4, !val);
        break;
    case 1:
        val = LOW_IOGET(GPIOB, 3);
        LOW_IOSET(GPIOB, 3, !val);
        break;

    }
}

#ifdef CONFIG_BOARD_COM_STDINOUT
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
int _write(int file, const char *ptr, int len)
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
size_t fwrite(const void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _write(stream->_file, ptr, size*n_items);
}
size_t fread(void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _read(stream->_file, ptr, size*n_items);
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

