#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <device/dnode.h>
#include <device/serial.h>
#include <device/i2c_master.h>
#include <device/spi.h>

/**************
 * TELEM1 Serial port
 **************/
uint8_t telem1_serial_dma_rxbuff[256];
uint8_t telem1_serial_dma_txbuff[256];
uint8_t telem1_serial_txbuff[512];
uint8_t telem1_serial_rxbuff[512];
struct up_uart_dev_s telem1_serial_dev = 
{
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 512,
            .buffer = telem1_serial_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = telem1_serial_txbuff,
        },
        .dmarx = {
            .capacity = 256,
            .buffer = telem1_serial_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = telem1_serial_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &telem1_serial_dev,
    },
    .id = 2,  //usart2
    .pin_tx = 1, // PD5 PD6 (1 for remap)
    .pin_rx = 0,
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/**************
 * Sensor SPI1 --- L3GD20 GYRO + LSM303D ACCEL+MAG + MPU6000 + MS5611-01A
 **************/
struct up_spi_dev_s sensor_spi_dev = 
{
    .dev = {
		.frequency = SPI_BAUDRATEPRESCALER_64,
		.mode = SPIDEV_MODE0,
		.nbits = 8,
        .ops       = &g_spi_ops,
        .priv      = &sensor_spi_dev,
    },
    .id = 1, //SPI1
	.pin_ncs = 0, //soft
	.pin_sck = 1, //PA5
	.pin_miso = 1, //PA6
	.pin_mosi = 1, //PA7
    .priority = 4,
    .priority_dmarx = 5,
    .priority_dmatx = 5,
    .enable_dmarx = true,
    .enable_dmatx = true,
	/*            L3GD20 GYRO    LSM303D ACCEL+MAG         MPU6000      MS5611-01A      */
    .devid = { [0] = 0x11,        [1] = 0x12,        [2] = 0x13,       [3] = 0x14,      },
	.devcs = { [0] = {GPIOC, 13}, [1] = {GPIOC, 15}, [2] = {GPIOC, 2}, [3] = {GPIOD, 7},},
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
	GPIO_InitTypeDef obj;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

	BOARD_INIT_IOPORT(0, GPIO_nLED_PORT, GPIO_nLED_PIN, GPIO_MODE_OUTPUT_PP, 
		GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

    BOARD_INIT_IOPORT(1, GPIO_VDD_5V_PERIPH_nEN_PORT, GPIO_VDD_5V_PERIPH_nEN_PIN, GPIO_MODE_OUTPUT_PP, 
        GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(2, GPIO_VDD_3V3_SENSORS_nEN_PORT, GPIO_VDD_3V3_SENSORS_nEN_PIN, GPIO_MODE_OUTPUT_PP, 
        GPIO_PULLDOWN, GPIO_SPEED_FREQ_VERY_HIGH);

    BOARD_INIT_IOPORT(3, GPIO_VDD_5V_SENS_OC_PORT, GPIO_VDD_5V_SENS_OC_PIN, GPIO_MODE_INPUT, 
        GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(4, GPIO_VDD_5V_HIPOWER_OC_PORT, GPIO_VDD_5V_HIPOWER_OC_PIN, GPIO_MODE_INPUT, 
        GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(5, GPIO_VDD_5V_PERIPH_OC_PORT, GPIO_VDD_5V_PERIPH_OC_PIN, GPIO_MODE_INPUT, 
        GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);

    BOARD_LED(false);
    VDD_5V_PERIPH_EN(true);
    VDD_3V3_SENSOR_EN(true);
    /* delay after sensor power enable */
    HAL_Delay(300); 

    // init spi soft cs pin
    BOARD_INIT_IOPORT(2, GPIOC, 2, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(3, GPIOC, 13, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(4, GPIOC, 15, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_INIT_IOPORT(5, GPIOD, 7, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    BOARD_IO_SET(GPIOC, 13, 1);
    BOARD_IO_SET(GPIOC, 15, 1);
    BOARD_IO_SET(GPIOC, 2, 1);
    BOARD_IO_SET(GPIOD, 7, 1);

    dregister("/telem1", &telem1_serial_dev.dev);
    dregister("/sensor_spi", &sensor_spi_dev.dev);
    dstdout = dbind("/telem1");
    dstdin = dbind("/telem1");
	telem1_serial_dev.dev.ops->setup(&telem1_serial_dev.dev);
	sensor_spi_dev.dev.ops->setup(&sensor_spi_dev.dev);

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
}

void board_debug()
{
    board_led_toggle();
}

void board_led_toggle()
{
    int val = BOARD_IO_GET(GPIO_nLED_PORT, GPIO_nLED_PIN);
    BOARD_IO_SET(GPIO_nLED_PORT, GPIO_nLED_PIN, !val);
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
