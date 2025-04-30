#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <device/dnode.h>
#include <device/serial.h>
#include <device/i2c_master.h>
#include <device/spi.h>

/**************
 * GPS1 Serial port
 **************/
uint8_t gps1_serial_dma_rxbuff[256];
uint8_t gps1_serial_dma_txbuff[256];
uint8_t gps1_serial_txbuff[512];
uint8_t gps1_serial_rxbuff[512];
struct up_uart_dev_s gps1_serial_dev = 
{
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 512,
            .buffer = gps1_serial_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = gps1_serial_txbuff,
        },
        .dmarx = {
            .capacity = 256,
            .buffer = gps1_serial_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = gps1_serial_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &gps1_serial_dev,
    },
    .id = 1,  //usart1
    .pin_tx = 2, // PB6
    .pin_rx = 1, // PA10
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/**************
 * Sensor I2C --- MS5611 + IST8310 + EEPROM
 **************/
struct up_i2c_master_s sensor_i2c_dev = 
{
    .dev = {
		.clk_speed = 400000,
		.addr_mode = I2C_ADDR_7BIT,
        .ops       = &g_i2c_master_ops,
        .priv      = &sensor_i2c_dev,
    },
	.id = 4, //i2c4
	.pin_scl = 3, //PD12
    .pin_sda = 3, //PD13
    .priority_event = 4,
    .priority_error = 5,
};

/**************
 * Sensor SPI1 --- BMI055 ACCEL + BMI GYRO + ICM-42688-P
 **************/
struct up_spi_dev_s sensor_spi_dev = 
{
    .dev = {
		.frequency = SPI_BAUDRATEPRESCALER_32,
		.mode = SPIDEV_MODE0,
		.nbits = 8,
        .ops       = &g_spi_ops,
        .priv      = &sensor_spi_dev,
    },
    .id = 1, //SPI1
	.pin_ncs = 1, //soft
	.pin_sck = 1, //PA5
	.pin_miso = 1, //PA6
	.pin_mosi = 1, //PA7
    .priority = 4,
    .priority_dmarx = 5,
    .priority_dmatx = 5,
    .enable_dmarx = true,
    .enable_dmatx = true,
	/*           BMI055-ACC   BMI-055 GYRO            ICM-42688 */
    .devid = { [0] = 0x11,        [1] = 0x12,        [2] = 0x13,        },
	.devcs = { [0] = {GPIOC, 15}, [1] = {GPIOC, 14}, [2] = {GPIOC, 13}, },
};

/**************
 * FRAM SPI2 --- FM25V02A-G
 **************/
struct up_spi_dev_s fram_spi_dev = 
{
    .dev = {
		.frequency = SPI_BAUDRATEPRESCALER_32,
		.mode = SPIDEV_MODE0,
		.nbits = 8,
        .ops       = &g_spi_ops,
        .priv      = &fram_spi_dev,
    },
    .id = 2, //SPI2
	.pin_ncs = 1, //soft
	.pin_sck = 6, //PD3
	.pin_miso = 1, //PC2
	.pin_mosi = 2, //PC3
    .priority = 4,
    .priority_dmarx = 5,
    .priority_dmatx = 5,
    .enable_dmarx = true,
    .enable_dmatx = true,
	/*           FM25V02A-G */
    .devid = { [0] = 0x21,        },
	.devcs = { [0] = {GPIOD, 4}, },
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
	GPIO_InitTypeDef obj;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

	// Red Led
	BOARD_INIT_IOPORT(0, GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN, GPIO_MODE_OUTPUT_PP, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

	// Blue Led
	BOARD_INIT_IOPORT(1, GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, GPIO_MODE_OUTPUT_PP, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

	// Power in detector
	BOARD_INIT_IOPORT(2, GPIO_nPOWER_IN_A_PORT, GPIO_nPOWER_IN_A_PIN, GPIO_MODE_INPUT, 
		GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
	BOARD_INIT_IOPORT(3, GPIO_nPOWER_IN_B_PORT, GPIO_nPOWER_IN_B_PIN, GPIO_MODE_INPUT, 
		GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
	BOARD_INIT_IOPORT(4, GPIO_nPOWER_IN_C_PORT, GPIO_nPOWER_IN_C_PIN, GPIO_MODE_INPUT, 
		GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);

	// periph output ctrl and end
	BOARD_INIT_IOPORT(5, GPIO_VDD_5V_PERIPH_nEN_PORT, GPIO_VDD_5V_PERIPH_nEN_PIN, GPIO_MODE_OUTPUT_PP, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	BOARD_INIT_IOPORT(6, GPIO_VDD_5V_PERIPH_nOC_PORT, GPIO_VDD_5V_PERIPH_nOC_PIN, GPIO_MODE_INPUT, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

	// hipower output ctrl and end
	BOARD_INIT_IOPORT(7, GPIO_VDD_5V_HIPOWER_nEN_PORT, GPIO_VDD_5V_HIPOWER_nEN_PIN, GPIO_MODE_OUTPUT_PP, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	BOARD_INIT_IOPORT(8, GPIO_VDD_5V_HIPOWER_nOC_PORT, GPIO_VDD_5V_HIPOWER_nOC_PIN, GPIO_MODE_INPUT, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

	BOARD_INIT_IOPORT(9, GPIO_VDD_3V3_SENSORS_EN_PORT, GPIO_VDD_3V3_SENSORS_EN_PIN, GPIO_MODE_OUTPUT_PP, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

	BOARD_INIT_IOPORT(10, GPIO_OTGFS_VBUS_PORT, GPIO_OTGFS_VBUS_PIN, GPIO_MODE_INPUT, 
		GPIO_PULLDOWN, GPIO_SPEED_FREQ_VERY_HIGH);

	VDD_5V_PERIPH_EN(true);
	VDD_5V_HIPOWER_EN(true);
    VDD_3V3_SENSORS_EN(true);

    /* delay after sensor power enable */
    HAL_Delay(200); 

	BOARD_BLUE_LED(false);
	BOARD_RED_LED(false);

    // spi soft cs pin
    BOARD_INIT_IOPORT(11, GPIOC, 13, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(12, GPIOC, 14, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(13, GPIOC, 15, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(14, GPIOD, 4, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

    dregister("/gps_serial", &gps1_serial_dev.dev);
    dregister("/sensor_i2c", &sensor_i2c_dev.dev);
    dregister("/sensor_spi", &sensor_spi_dev.dev);
    dregister("/fram_spi", &fram_spi_dev.dev);

	gps1_serial_dev.dev.ops->setup(&gps1_serial_dev.dev);
    sensor_i2c_dev.dev.ops->setup(&sensor_i2c_dev.dev);
    sensor_spi_dev.dev.ops->setup(&sensor_spi_dev.dev);
	fram_spi_dev.dev.ops->setup(&fram_spi_dev.dev);

    dstdout = dbind("/gps_serial");
    dstdin = dbind("/gps_serial");

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
}

void board_blue_led_toggle()
{
    int val = BOARD_IO_GET(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN);
    BOARD_IO_SET(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, !val);
}

void board_red_led_toggle()
{
    int val = BOARD_IO_GET(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN);
    BOARD_IO_SET(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN, !val);
}

void board_debug()
{
    board_blue_led_toggle();
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
