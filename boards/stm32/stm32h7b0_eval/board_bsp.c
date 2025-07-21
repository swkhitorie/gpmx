#include "board_config.h"
#include <drv_uart.h>
#include <drv_i2c.h>
#include <device/dnode.h>
#include <device/serial.h>
#include <device/i2c_master.h>

/* COM5 --- for output debug in TH1030 */
uint8_t com5_dma_rxbuff[256];
uint8_t com5_dma_txbuff[256];
uint8_t com5_txbuff[512];
uint8_t com5_rxbuff[512];
struct up_uart_dev_s com5_dev = {
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 512,
            .buffer = com5_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com5_txbuff,
        },
        .dmarx = {
            .capacity = 256,
            .buffer = com5_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com5_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com5_dev,
    },
    .id = 5,
    .pin_tx = 3,  //PB13
    .pin_rx = 3,  //PB12
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/* COM7 --- connect to sta8135 */
uint8_t com7_dma_rxbuff[1024];
uint8_t com7_dma_txbuff[256];
uint8_t com7_txbuff[512];
uint8_t com7_rxbuff[1024];
struct up_uart_dev_s com7_dev = {
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 1024,
            .buffer = com7_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com7_txbuff,
        },
        .dmarx = {
            .capacity = 1024,
            .buffer = com7_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com7_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com7_dev,
    },
    .id = 7,
    .pin_tx = 4,  //PB4
    .pin_rx = 4,  //PB3
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
		.clk_speed = 100000,   //100kHZ
		.addr_mode = I2C_ADDR_7BIT,
        .ops       = &g_i2c_master_ops,
        .priv      = &sensor_i2c_dev,
    },
	.id = 2, //i2c2
	.pin_scl = 1, //PB10
    .pin_sda = 1, //PB11
    .priority_event = 4,
    .priority_error = 5,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
    dregister("/com5", &com5_dev.dev);
    dregister("/com7", &com7_dev.dev);
    dregister("/sensor_i2c", &sensor_i2c_dev.dev);

    com5_dev.dev.ops->setup(&com5_dev.dev);
    com7_dev.dev.ops->setup(&com7_dev.dev);
    sensor_i2c_dev.dev.ops->setup(&sensor_i2c_dev.dev);

    dstdout = dbind("/com5");
    dstdin = dbind("/com5");
}

uint8_t buff_debug[1024];
void board_debug()
{
    // int size = SERIAL_RDBUF(dstdin, buff_debug, 256);
    // if (size > 0) {
    //     for (int i = 0; i < size; i++) {
    //         printf("%c", buff_debug[i]);
    //     }
    //     printf("\r\n");
    // }
    printf("hello h7b0 \r\n");

    // int size = SERIAL_RDBUF(gnss_module, buff_debug, 1024);
    // if (size > 0) {
    //     for (int i = 0; i < size; i++) {
    //         printf("%x", buff_debug[i]);
    //     }
    //     printf("\r\n");
    // }
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

#ifdef CONFIG_BOARD_HRT_TIMEBASE
#include <drivers/drv_hrt.h>
hrt_abstime hrt_absolute_time(void)
{
    // uint64_t m0 = HAL_GetTick();
    // volatile uint64_t u0 = SysTick->VAL;
    // const uint64_t tms = SysTick->LOAD + 1;
    // volatile uint64_t abs_time = (m0 * 1000 + ((tms - u0) * 1000) / tms);
    // return abs_time;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t m = HAL_GetTick();
    volatile uint32_t v = SysTick->VAL;
    // If an overflow happened since we disabled irqs, it cannot have been
    // processed yet, so increment m and reload VAL to ensure we get the
    // post-overflow value.
    if (SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) {
        ++m;
        v = SysTick->VAL;
    }

    // Restore irq status
    __set_PRIMASK(primask);

    const uint32_t tms = SysTick->LOAD + 1;
    return (m * 1000 + ((tms - v) * 1000) / tms);
}
#endif
