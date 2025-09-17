#include <board_config.h>
#include <drv_gpio.h>
#include <drv_uart.h>
#include <drv_spi.h>

static uint8_t module_rxdma_buffer[800];
static uint8_t module_rx_buffer[1024];

struct n32_uart_dev tty_module = {
    .id = 4,
    .baud = 460800,
    .wordlen = USART_WL_8B,
    .stopbits = USART_STPB_1,
    .parity = USART_PE_NO,

    .priority = 2,
    .rxdma_channel = DMA_CH4,
    .rxdma_buffer = module_rxdma_buffer,
    .rxdma_size = 800,

    .rxbuf = {
        .in = 0,
        .out = 0,
        .size = 0,
        .capacity = 1024,
        .buffer = (uint8_t *)&module_rx_buffer[0],
    },

    .txport = GPIOB,
    .txpin = 0,
    .txalternate = GPIO_AF7_UART4,
    
    .rxport = GPIOB,
    .rxpin = 1,
    .rxalternate = GPIO_AF7_UART4,
};

static uint8_t shlog_rxdma_buffer[64];
static uint8_t shlog_rx_buffer[80];
struct n32_uart_dev tty_shlog = {
    .id = 1,
    .baud = 115200,
    .wordlen = USART_WL_8B,
    .stopbits = USART_STPB_1,
    .parity = USART_PE_NO,

    .priority = 4,
    .rxdma_channel = DMA_CH1,
    .rxdma_buffer = shlog_rxdma_buffer,
    .rxdma_size = 64,

    .rxbuf = {
        .in = 0,
        .out = 0,
        .size = 0,
        .capacity = 80,
        .buffer = (uint8_t *)&shlog_rx_buffer[0],
    },

    .txport = GPIOA,
    .txpin = 9,
    .txalternate = GPIO_AF5_USART1,
    
    .rxport = GPIOA,
    .rxpin = 10,
    .rxalternate = GPIO_AF5_USART1,
};

struct n32_spimaster_dev lora_dev = {
    .id = 1,
    .mode = 0,
    .words = 8,

    .clk_port = GPIOB,
    .clk_pin = 3,
    .clk_alternate = GPIO_AF2_SPI1,

    .mosi_port = GPIOB,
    .mosi_pin = 5,
    .mosi_alternate = GPIO_AF1_SPI1,

    .miso_port = GPIOB,
    .miso_pin = 4,
    .miso_alternate = GPIO_AF2_SPI1,

    .cs_port = GPIOB,
    .cs_pin = 6,
};

void board_bsp_init()
{
    // LED Init
//    LOW_INITPIN(GPIOA, 0, GPIO_MODE_OUT_PP, GPIO_PULL_UP, GPIO_SLEW_RATE_FAST);
//    LOW_INITPIN(GPIOA, 1, GPIO_MODE_OUT_PP, GPIO_PULL_UP, GPIO_SLEW_RATE_FAST);
    LOW_INITPIN(GPIOA, 2, GPIO_MODE_OUT_PP, GPIO_PULL_UP, GPIO_SLEW_RATE_FAST);
//    n32_pin_set(GPIOA, 0, 1);
//    n32_pin_set(GPIOA, 1, 1);
    n32_pin_set(GPIOA, 2, 1);

    n32_uart_init(&tty_shlog);
    n32_uart_init(&tty_module);
    n32_spimaster_init(&lora_dev);
}

void board_get_uid(uint32_t *p)
{
    p[0] = *(volatile uint32_t*)(0x1FFFF7F0);
    p[1] = *(volatile uint32_t*)(0x1FFFF7F4);
    p[2] = *(volatile uint32_t*)(0x1FFFF7F8);
}

void board_led_toggle(int idx)
{
    //printf("tty rx size: %d \r\n", ringbuf_size(&tty_module.rxbuf));
    switch (idx) {
    case 0: n32_pin_toggle(GPIOA, 0); break;
    case 1: n32_pin_toggle(GPIOA, 1); break;
    case 2: n32_pin_toggle(GPIOA, 2); break;
    }
}

uint8_t board_spi_exchange_byte(uint8_t val)
{
    return n32_spimaster_exchange_byte(&lora_dev, val);
}

uint16_t board_tty_module_read(uint8_t *p, uint16_t len)
{
    return rbdrv_read(&tty_module.rxbuf, p, len);    
}

uint16_t board_tty_module_send(uint8_t *p, uint16_t len)
{
    n32_uart_sendblk(&tty_module, p, len);
    return len;
}

uint32_t board_rng_get()
{
    return SysTick->VAL*654+642;    
}

#include <stdio.h>
int fputc(int ch, FILE* f)
{
    n32_uart_sendblk(&tty_shlog, (uint8_t *)&ch, 1);
    return ch;
}

