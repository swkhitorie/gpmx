#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>

#include "mpu6050_test.h"


#ifdef BSP_COM_PRINTF
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
#endif

struct drv_uart_t com1;
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];

struct drv_i2c_reg_cmd i2c_cmdlist_mem[8];
struct drv_i2c_t i2c;

void board_bsp_init()
{
    // wait all peripheral power on
    HAL_Delay(500);
    struct drv_uart_attr_t com1_attr;
    struct drv_uart_dma_attr_t com1_rxdma_attr;
    struct drv_uart_dma_attr_t com1_txdma_attr;
    drv_uart_dma_attr_init(&com1_rxdma_attr, &com1_dma_rxbuff[0], 256, 2);
    drv_uart_dma_attr_init(&com1_txdma_attr, &com1_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com1_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 1);
    drv_uart_buff_init(&com1, &com1_txbuff[0], 512, &com1_rxbuff[0], 512);
    drv_uart_init(1, &com1, 0, 0, &com1_attr, &com1_txdma_attr, &com1_rxdma_attr);

    struct drv_i2c_attr_t i2c_attr;
    struct drv_i2c_reg_cmdlist i2c_buff;
    drv_i2c_attr_config(&i2c_attr, 100000, 0x01, 0x02);
    drv_i2c_cmdlist_config(&i2c_buff, &i2c_cmdlist_mem[0], 8);
    drv_i2c_config(2, 0, 0, &i2c, &i2c_attr, &i2c_buff);
    drv_i2c_init(&i2c);

    if (mpu6050_init()) {
        printf("mpu6050 init success \r\n");
    } else {
        printf("mpu6050 init failed \r\n");
    }
}

//uint8_t buff_debug[256];
void board_debug()
{
    // devbuf_t buf = drv_uart_devbuf(&com1);
    // int size = devbuf_size(&buf);
    // if (size > 0) {
    //     fprintf(stdout, "[%d,%d] ", HAL_GetTick(),size);
    //     devbuf_read(&com1.rx_buf, &buff_debug[0], size);
    //     for (int i = 0; i < size; i++) {
    //         fprintf(stdout, "%d ", buff_debug[i]);
    //     }
    //     fprintf(stdout, "\r\n");
    // }
    //printf("hello world f407\r\n");
    int16_t accel[3];
    mpu6050_accel(&accel[0]);
    printf("mpu accel: %d, %d, %d \r\n", accel[0], accel[1], accel[2]);
}

#ifdef BSP_COM_PRINTF
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
#endif

void write_register(uint8_t addr, uint8_t data)
{
    drv_i2c_reg_write(&i2c, 0xD0, addr, I2CMEMADD_8BITS, &data, 1, RWPOLL);
}

void read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway)
{
    if (rwway == 0) {
        drv_i2c_reg_read(&i2c, 0xD0, addr, I2CMEMADD_8BITS, buf, len, RWPOLL);
    } else {
        drv_i2c_reg_read(&i2c, 0xD0, addr, I2CMEMADD_8BITS, buf, len, RWIT);
    }
    return;
}

