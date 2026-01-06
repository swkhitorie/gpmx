#include <board_config.h>
#include <drv_uart.h>
#include <device/dnode.h>
#include <device/serial.h>

#include <stdarg.h>

#if defined(CONFIG_BOARD_CMBACKTRACE)
#include "cm_backtrace.h"
#endif

#if defined(CONFIG_BOARD_EMBEDPRINTF)
#include <lib_eprintf.h>
#endif

#if defined(SERIAL1_CONFIG)
uint8_t serial1_txdma_bufer[SERIAL1_TXBUFFER_DMA_SIZE];
uint8_t serial1_rxdma_bufer[SERIAL1_RXBUFFER_DMA_SIZE];
uint8_t serial1_tx_bufer[SERIAL1_TXBUFFER_SIZE];
uint8_t serial1_rx_bufer[SERIAL1_RXBUFFER_SIZE];
#else
#define SERIAL1_TXBUFFER_DMA_SIZE  (128)
#define SERIAL1_RXBUFFER_DMA_SIZE  (128)
#define SERIAL1_TXBUFFER_SIZE      (64)
#define SERIAL1_RXBUFFER_SIZE      (64)
uint8_t serial1_txdma_bufer[SERIAL1_TXBUFFER_DMA_SIZE];
uint8_t serial1_rxdma_bufer[SERIAL1_RXBUFFER_DMA_SIZE];
uint8_t serial1_tx_bufer[SERIAL1_TXBUFFER_SIZE];
uint8_t serial1_rx_bufer[SERIAL1_RXBUFFER_SIZE];
#endif

#if defined(SERIAL2_CONFIG)
uint8_t serial2_txdma_bufer[SERIAL2_TXBUFFER_DMA_SIZE];
uint8_t serial2_rxdma_bufer[SERIAL2_RXBUFFER_DMA_SIZE];
uint8_t serial2_tx_bufer[SERIAL2_TXBUFFER_SIZE];
uint8_t serial2_rx_bufer[SERIAL2_RXBUFFER_SIZE];
#else
#define SERIAL2_TXBUFFER_DMA_SIZE  (128)
#define SERIAL2_RXBUFFER_DMA_SIZE  (128)
#define SERIAL2_TXBUFFER_SIZE      (64)
#define SERIAL2_RXBUFFER_SIZE      (64)
uint8_t serial2_txdma_bufer[SERIAL2_TXBUFFER_DMA_SIZE];
uint8_t serial2_rxdma_bufer[SERIAL2_RXBUFFER_DMA_SIZE];
uint8_t serial2_tx_bufer[SERIAL2_TXBUFFER_SIZE];
uint8_t serial2_rx_bufer[SERIAL2_RXBUFFER_SIZE];
#endif

#if !defined(SERIAL1_BAUD)
#define SERIAL1_BAUD      (115200)
#endif

#if !defined(SERIAL2_BAUD)
#define SERIAL2_BAUD      (115200)
#endif

struct up_uart_dev_s com2_dev = {
    .dev = {
        .baudrate = SERIAL2_BAUD,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = SERIAL2_RXBUFFER_SIZE,
            .buffer = serial2_rx_bufer,
        },
        .xmit = {
            .capacity = SERIAL2_TXBUFFER_SIZE,
            .buffer = serial2_tx_bufer,
        },
        .dmarx = {
            .capacity = SERIAL2_RXBUFFER_DMA_SIZE,
            .buffer = serial2_rxdma_bufer,
        },
        .dmatx = {
            .capacity = SERIAL2_TXBUFFER_DMA_SIZE,
            .buffer = serial2_txdma_bufer,
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
        .baudrate = SERIAL1_BAUD,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = SERIAL1_RXBUFFER_SIZE,
            .buffer = serial1_rx_bufer,
        },
        .xmit = {
            .capacity = SERIAL1_TXBUFFER_SIZE,
            .buffer = serial1_tx_bufer,
        },
        .dmarx = {
            .capacity = SERIAL1_RXBUFFER_DMA_SIZE,
            .buffer = serial1_rxdma_bufer,
        },
        .dmatx = {
            .capacity = SERIAL1_TXBUFFER_DMA_SIZE,
            .buffer = serial1_txdma_bufer,
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

#if defined(CONFIG_BOARD_PRINTF_SOURCE)
uart_dev_t *dstdout;
#endif

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(200);

    /* LED Array */
    LOW_INITPIN(GPIOB, 15, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOB, 9, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOB, 11, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    serial_register(&com1_dev.dev, 1);
    serial_register(&com2_dev.dev, 2);

    serial_bus_initialize(1);
    serial_bus_initialize(2);

#if defined(CONFIG_BOARD_PRINTF_SOURCE)
    dstdout = serial_bus_get(CONFIG_BOARD_PRINTF_SOURCE);
#endif

#if defined(CONFIG_BOARD_CMBACKTRACE)
    cm_backtrace_init(BOARD_FIRMWARE_NAME, BOARD_HARDWARE_VERSION, BOARD_SOFTWARE_VERSION);
#endif

    SERIAL_RXCLEAR(&com1_dev.dev);
    SERIAL_RXCLEAR(&com2_dev.dev);

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

void board_led_toggle(uint8_t i)
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

void board_get_uid(uint32_t *p)
{
    p[0] = *(volatile uint32_t*)(0x1FFF7590);
    p[1] = *(volatile uint32_t*)(0x1FFF7594);
    p[2] = *(volatile uint32_t*)(0x1FFF7598);
}

/****************************************************************************
 * Board Tickms interface
 ****************************************************************************/
uint32_t board_get_time()
{
    return HAL_GetTick();
}

void board_delay(uint32_t ms)
{
    HAL_Delay(ms);
}

uint32_t board_elapsed_time(const uint32_t timestamp)
{
    uint32_t now = HAL_GetTick();
    if (timestamp > now) {
        return 0;
    }
    return now - timestamp;
}

/****************************************************************************
 * Board Stream serial/usb interface
 ****************************************************************************/
void board_stream_outc(char character, void* arg)
{
    int port = *(int *)arg;
    board_stream_out(port, &character, 1, 0);
}

int board_stream_in(int port, void *p, int size)
{
    switch (port) {
    case 0: return SERIAL_RDBUF(&com1_dev.dev, p, size);
    case 1: return SERIAL_RDBUF(&com2_dev.dev, p, size);
    }
    return 0;
}

int board_stream_out(int port, const void *p, int size, int way)
{
    switch (port) {
    case 0: {
            if (way == 0) {
                return SERIAL_SEND(&com1_dev.dev, p, size);
            } else {
                return SERIAL_DMASEND(&com1_dev.dev, p, size);
            }
        }
    case 1: {
            if (way == 0) {
                return SERIAL_SEND(&com2_dev.dev, p, size);
            } else {
                return SERIAL_DMASEND(&com2_dev.dev, p, size);
            }
        }
    }
    return 0;
}

void board_stream_printf(int port, const char *format, ...)
{
    int idx;
    int iport = port;
    va_list args;

#if defined(CONFIG_BOARD_STDPRINTF)
    char tmp_buffer[512];
    va_start(args, format);
    idx = vsnprintf(tmp_buffer, 512, format, args);
    if (idx > 512) {
        idx = 512;
    }
    board_stream_out(port, tmp_buffer, idx, 0);
#endif

#if defined(CONFIG_BOARD_EMBEDPRINTF)
    va_start(args, format);
    vfctprintf_(board_stream_outc, &iport, format, args);
#endif

    va_end(args);
}

/****************************************************************************
 * Board printf setting
 ****************************************************************************/
#if defined(CONFIG_BOARD_STDPRINTF)
#include <string.h>
#include <stdio.h>

#if defined (__CC_ARM) || defined(__ARMCC_VERSION)

int fputc(int c, FILE *f)
{
#if defined(CONFIG_BOARD_PRINTF_SOURCE)
    SERIAL_SEND(dstdout, ptr, len);
#endif
}
#elif defined(__GNUC__)

int _write(int file, const char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
#if defined(CONFIG_BOARD_PRINTF_SOURCE)
        SERIAL_SEND(dstdout, ptr, len);
#endif
    }

    return len;
}
#endif
#endif

#if defined(CONFIG_BOARD_EMBEDPRINTF)
void _putchar(char ch)
{
#if defined(CONFIG_BOARD_PRINTF_SOURCE)
    SERIAL_SEND(dstdout, &ch, 1);
#endif
}
#endif

void board_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

#if defined(CONFIG_BOARD_STDPRINTF)
    vprintf(format, args);
#endif

#if defined(CONFIG_BOARD_EMBEDPRINTF)
    vprintf_(format, args);
#endif

    va_end(args);
}
