#include <board_config.h>
#include <drv_uart.h>
#include <drv_spi.h>

#include "l3gd20_test.h"

#ifdef BSP_COM_PRINTF
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
#endif

static void board_config_io();

struct drv_uart_t com2;
uint8_t com2_dma_rxbuff[256];
uint8_t com2_dma_txbuff[256];
uint8_t com2_txbuff[512];
uint8_t com2_rxbuff[512];

struct drv_spi_t spi;
struct drv_pin_t spi_cs;

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(500);
    board_config_io();
    BOARD_LED(false);
    VDD_5V_PERIPH_EN(true);

    struct drv_uart_attr_t com2_attr;
    struct drv_uart_dma_attr_t com2_rxdma_attr;
    struct drv_uart_dma_attr_t com2_txdma_attr;
    drv_uart_dma_attr_init(&com2_rxdma_attr, &com2_dma_rxbuff[0], 256, 2);
    drv_uart_dma_attr_init(&com2_txdma_attr, &com2_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com2_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 1);
    drv_uart_buff_init(&com2, &com2_txbuff[0], 512, &com2_rxbuff[0], 512);
    drv_uart_init(2, &com2, 1, 0, &com2_attr, &com2_txdma_attr, &com2_rxdma_attr);

    struct drv_spi_attr_t spi_attr;
    drv_spi_attr_init(&spi_attr, SPI_MODE0, SPI_8BITS, SPI_PRESCAL_32);
    drv_spi_init(1, &spi, &spi_attr, 0, 0, 0, 0);
    spi_cs = drv_gpio_init(GPIOC, 13, IOMODE_OUTPP, IO_SPEEDHIGH, IO_PULLUP, 0, NULL);

    // spi not available
    l3gd20_init();

#ifdef BSP_MODULE_USB_CHERRY
    HAL_Delay(600);
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
}

int16_t gyro_data[3];
void board_debug()
{
    // printf("[fmuv2] usart2 printf debug dma \r\n");
    //int ret = l3gd20_read(&gyro_data[0]);
    //printf("[l3gd20] %d %d %d \r\n", gyro_data[0], gyro_data[1], gyro_data[2]);
    l3gd20_init();
    board_led_toggle();
}

void board_led_toggle()
{
	int val = HAL_GPIO_ReadPin(GPIO_nLED_PORT, GPIO_nLED_PIN);
	HAL_GPIO_WritePin(GPIO_nLED_PORT, GPIO_nLED_PIN, !val);
}

void board_config_io()
{
	GPIO_InitTypeDef obj;
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    obj.Pin = GPIO_nLED_PIN;
    obj.Mode = GPIO_MODE_OUTPUT_PP;
    obj.Pull = GPIO_NOPULL;
    obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIO_nLED_PORT, &obj);

    obj.Pin = GPIO_VDD_5V_PERIPH_nEN_PIN;
    obj.Mode = GPIO_MODE_OUTPUT_PP;
    obj.Pull = GPIO_NOPULL;
    obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIO_VDD_5V_PERIPH_nEN_PORT, &obj);
}

#ifdef BSP_COM_PRINTF
int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
        drv_uart_send(&com2, ptr, len, RWDMA);
    }
    return len;
}
#endif

void l3gd20_write_register(uint8_t addr, uint8_t data)
{
    int ret = 0;
	static uint8_t dx[2];
	dx[0] = addr & ~0x80;
	dx[1] = data;

    drv_gpio_write(&spi_cs, 0);
	ret = drv_spi_write(&spi, &dx[0], 2, RWPOLL);
	drv_gpio_write(&spi_cs, 1);
}

void l3gd20_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway)
{
    int ret = 0;

	uint8_t send_addr = addr | 0x80;

    drv_gpio_write(&spi_cs, 0);
    // ret = drv_spi_readwrite(&spi, &send_addr, buf, 1, RWPOLL);
	ret = drv_spi_write(&spi, &send_addr, 1, RWPOLL);
    ret = drv_spi_read(&spi, buf, len, RWPOLL);
	drv_gpio_write(&spi_cs, 1);
}

