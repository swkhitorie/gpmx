#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>

#include "ist8310_test.h"
#include "icm42688_test.h"

static void board_config_io();

struct drv_uart_t com1;
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];

struct drv_i2c_reg_cmd i2c_cmdlist_mem[8];
struct drv_i2c_t i2c;

struct drv_spi_t spi;
struct drv_pin_t spi_cs;

void board_bsp_init()
{
    board_config_io();

	VDD_5V_PERIPH_EN(true);
	VDD_5V_HIPOWER_EN(true);
    VDD_3V3_SENSORS_EN(true);

	BOARD_BLUE_LED(false);
	BOARD_RED_LED(false);

    struct drv_uart_attr_t com1_attr;
    struct drv_uart_dma_attr_t com1_rxdma_attr;
    struct drv_uart_dma_attr_t com1_txdma_attr;
    drv_uart_dma_attr_init(&com1_rxdma_attr, &com1_dma_rxbuff[0], 256, 2);
    drv_uart_dma_attr_init(&com1_txdma_attr, &com1_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com1_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 1);
    drv_uart_buff_init(&com1, &com1_txbuff[0], 512, &com1_rxbuff[0], 512);
    drv_uart_init(1, &com1, 2, 1, &com1_attr, &com1_txdma_attr, &com1_rxdma_attr);
    //drv_uart_init(1, &com1, 2, 1, &com1_attr, NULL, NULL);

    struct drv_i2c_attr_t i2c_attr;
    struct drv_i2c_reg_cmdlist i2c_buff;
    drv_i2c_attr_config(&i2c_attr, 400000, 0x01, 0x02);
    drv_i2c_cmdlist_config(&i2c_buff, &i2c_cmdlist_mem[0], 8);
    drv_i2c_config(4, 3, 3, &i2c, &i2c_attr, &i2c_buff);
    drv_i2c_init(&i2c);

    struct drv_spi_attr_t spi_attr;
    drv_spi_attr_init(&spi_attr, SPI_MODE0, SPI_8BITS, SPI_PRESCAL_32);
    drv_spi_init(1, &spi, &spi_attr, 1, 1, 1, 1);
    spi_cs = drv_gpio_init(GPIOC, 13, IOMODE_OUTPP, IO_SPEEDHIGH, IO_NOPULL, 0, NULL);

    if (ist8310_init()) {
        printf("ist8310 init success \r\n");
    } else {
        printf("ist8310 init failed \r\n");
    }

    icm42688_init();

#ifdef BSP_MODULE_USB_CHERRY
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
}

void board_config_io()
{
	GPIO_InitTypeDef obj;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

	// Red Led
	obj.Pin = GPIO_nLED_RED_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIO_nLED_RED_PORT, &obj);

	// Blue Led
	obj.Pin = GPIO_nLED_BLUE_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	obj.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIO_nLED_BLUE_PORT, &obj);

	// power in detector
	obj.Pin = GPIO_nPOWER_IN_A_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIO_nPOWER_IN_A_PORT, &obj);
	obj.Pin = GPIO_nPOWER_IN_B_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIO_nPOWER_IN_B_PORT, &obj);
	obj.Pin = GPIO_nPOWER_IN_C_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIO_nPOWER_IN_C_PORT, &obj);

	// periph output ctrl and end
	obj.Pin = GPIO_VDD_5V_PERIPH_nEN_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_PERIPH_nEN_PORT, &obj);
	obj.Pin = GPIO_VDD_5V_PERIPH_nOC_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_PERIPH_nOC_PORT, &obj);

	// hipower output ctrl and end
	obj.Pin = GPIO_VDD_5V_HIPOWER_nEN_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_HIPOWER_nEN_PORT, &obj);
	obj.Pin = GPIO_VDD_5V_HIPOWER_nOC_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_5V_HIPOWER_nOC_PORT, &obj);

	obj.Pin = GPIO_VDD_3V3_SENSORS_EN_PIN;
	obj.Mode = GPIO_MODE_OUTPUT_PP;
	obj.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_VDD_3V3_SENSORS_EN_PORT, &obj);

	obj.Pin = GPIO_OTGFS_VBUS_PIN;
	obj.Mode = GPIO_MODE_INPUT;
	obj.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIO_OTGFS_VBUS_PORT, &obj);
}

void board_blue_led_toggle()
{
	int val = HAL_GPIO_ReadPin(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN);
	HAL_GPIO_WritePin(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, !val);
}

void board_red_led_toggle()
{
	int val = HAL_GPIO_ReadPin(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN);
	HAL_GPIO_WritePin(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN, !val);
}

int16_t mag_data[3];
int16_t accel_data[3];

void board_debug()
{
	// printf("[v6c] usart test %d %d %d\r\n",
    //     BOARD_ADC_HIPOWER_5V_OC, BOARD_ADC_PERIPH_5V_OC, i2c.i2c_error_cnt);
	//ist8310_mag(mag_data);
	//printf("[ist8310] %d %d %d \r\n", mag_data[0], mag_data[1], mag_data[2]);
    int ret = icm42688_read(&accel_data[0]);
    printf("[icm42688] %d %d %d \r\n", accel_data[0], accel_data[1], accel_data[2]);
    board_blue_led_toggle();
}

#ifdef BSP_COM_PRINTF

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
        drv_uart_send(&com1, ptr, len, RWDMA);
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
    devbuf_t buf = drv_uart_devbuf(&com1);
    size_t rcv_size = devbuf_size(&buf);
    size_t sld_size = (len >= rcv_size) ? rcv_size: len;
    size_t ret_size = 0;
    if (file == stdin_fileno) {
        ret_size = devbuf_read(&com1.rx_buf, ptr, sld_size);
    }
    return ret_size;
}
#endif

void ist8310_write_register(uint8_t addr, uint8_t data)
{
    int ret = 0;
	ret = drv_i2c_reg_write(&i2c, 0x0c<<1, addr, I2CMEMADD_8BITS, &data, 1, RWPOLL);
}

void ist8310_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway)
{
	int ret = 0;
    if (rwway == 0) {
        ret = drv_i2c_reg_read(&i2c, 0x0c<<1, addr, I2CMEMADD_8BITS, buf, len, RWPOLL);
    } else {
        ret = drv_i2c_reg_read(&i2c, 0x0c<<1, addr, I2CMEMADD_8BITS, buf, len, RWIT);
    }
    return;
}

void icm42688_write_register(uint8_t addr, uint8_t data)
{
    int ret = 0;
	static uint8_t dx[2];
	dx[0] = addr & ~0x80;
	dx[1] = data;

    drv_gpio_write(&spi_cs, 0);
	ret = drv_spi_write(&spi, &dx[0], 2, RWPOLL);
	drv_gpio_write(&spi_cs, 1);
}

void icm42688_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway)
{
    int ret = 0;

	uint8_t send_addr = addr | 0x80;

    drv_gpio_write(&spi_cs, 0);
	ret = drv_spi_write(&spi, &send_addr, 1, RWPOLL);
    ret = drv_spi_read(&spi, buf, len, RWPOLL);

	drv_gpio_write(&spi_cs, 1);
}

