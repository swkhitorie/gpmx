#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <device/dnode.h>
#include <device/serial.h>
#include <device/i2c_master.h>

/* COM1 */
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 512,
            .buffer = com1_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com1_txbuff,
        },
        .dmarx = {
            .capacity = 256,
            .buffer = com1_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com1_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com1_dev,
    },
    .id = 1, //usart1
    .pin_tx = 0, //PA9
    .pin_rx = 0, //PA10
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/**************
 * Sensor I2C --- MPU6050
 **************/
struct up_i2c_master_s sensor_i2c_dev = 
{
    .dev = {
		.clk_speed = 100000,
		.addr_mode = I2C_ADDR_7BIT,
        .ops       = &g_i2c_master_ops,
        .priv      = &sensor_i2c_dev,
    },
	.id = 2, //i2c2
	.pin_scl = 0, //PB10
    .pin_sda = 0, //PB11
    .priority_event = 4,
    .priority_error = 5,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(200);

    dregister("/com1", &com1_dev.dev);
    dregister("/sensor_i2c", &sensor_i2c_dev.dev);

    com1_dev.dev.ops->setup(&com1_dev.dev);
    sensor_i2c_dev.dev.ops->setup(&sensor_i2c_dev.dev);

    dstdout = dbind("/com1");
    dstdin = dbind("/com1");
}

void board_debug()
{
    // printf("hello world f407\r\n");
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
        SERIAL_DMASEND(dstdout, ptr, len);
#else
        SERIAL_SEND(dstdout, ptr, len);
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
        rsize = SERIAL_RDBUF(dstdin, ptr, len);
    }
    return rsize;
}
#endif


