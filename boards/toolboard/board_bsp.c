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

/* COM2 */
uint8_t com2_dma_rxbuff[256];
uint8_t com2_dma_txbuff[256];
uint8_t com2_txbuff[512];
uint8_t com2_rxbuff[512];
struct up_uart_dev_s com2_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 512,
            .buffer = com2_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com2_txbuff,
        },
        .dmarx = {
            .capacity = 256,
            .buffer = com2_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com2_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com2_dev,
    },
    .id = 2,
    .txpin = { .port = GPIOA, .pin = 2, },
    .rxpin = { .port = GPIOA, .pin = 3, },
    .txdma_cfg = {
        .instance = DMA1_Channel7,
        .dma_rcc = RCC_AHBENR_DMA1EN,
        .dma_irq = DMA1_Channel7_IRQn,
        .priority = 2,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA1_Channel6,
        .dma_rcc = RCC_AHBENR_DMA1EN,
        .dma_irq = DMA1_Channel6_IRQn,
        .priority = 1,
        .enable = true,
    },
    .priority = 1,
};

#if (CONFIG_BOARD_PRINTF_SOURCE == 1)
uart_dev_t *dstdout;
#endif

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(100);
    LOW_INITPIN(GPIOB, 9, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    serial_register(&com2_dev.dev, 2);
    serial_bus_initialize(2);

#if (CONFIG_BOARD_PRINTF_SOURCE == 1)
    dstdout = serial_bus_get(2);
#endif

#if defined(CONFIG_BOARD_CMBACKTRACE)
    cm_backtrace_init(BOARD_FIRMWARE_NAME, BOARD_HARDWARE_VERSION, BOARD_SOFTWARE_VERSION);
#endif
}

void board_bsp_deinit()
{
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();

    __HAL_RCC_USART2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
    HAL_DMA_DeInit(com2_dev.com.hdmatx);
    HAL_DMA_DeInit(com2_dev.com.hdmarx);
    HAL_NVIC_DisableIRQ(USART2_IRQn);

    HAL_DeInit();
}

void board_led_toggle(uint8_t idx)
{
    (void)idx;
}

void board_get_uid(uint32_t *p)
{
    p[0] = *(volatile uint32_t*)(0x1FFFF7E8);
    p[1] = *(volatile uint32_t*)(0x1FFFF7EC);
    p[2] = *(volatile uint32_t*)(0x1FFFF7F0);
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
    case 0: return SERIAL_RDBUF(&com2_dev.dev, p, size);
    }
    return 0;
}

int board_stream_out(int port, const void *p, int size, int way)
{
    switch (port) {
    case 0: {
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
#if (CONFIG_BOARD_PRINTF_SOURCE == 1)
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
#if (CONFIG_BOARD_PRINTF_SOURCE == 1)
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
#if (CONFIG_BOARD_PRINTF_SOURCE == 1)
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

void board_debug()
{
    int val = LOW_IOGET(GPIOB, 9);
    LOW_IOSET(GPIOB, 9, !val);
}
