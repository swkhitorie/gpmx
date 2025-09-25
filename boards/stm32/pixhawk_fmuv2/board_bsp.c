#include <board_config.h>

#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <drv_can.h>
#include <drv_mmcsd.h>
#include <device/dnode.h>

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
#include "dev_cdc_acm.h"
#endif

/**************
 * serial2 [INTERFACE - TELEM1]
 **************/
uint8_t com2_dma_rxbuff[256];
uint8_t com2_dma_txbuff[256];
uint8_t com2_txbuff[512];
uint8_t com2_rxbuff[512];
struct up_uart_dev_s com2_dev = 
{
    .dev = {
        .baudrate = 460800,
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
    .txpin = { .port = GPIOD, .pin = 5,  .alternate = GPIO_AF7_USART2,},
    .rxpin = { .port = GPIOD, .pin = 6, . alternate = GPIO_AF7_USART2,},
    .txdma_cfg = {
        .instance = DMA1_Stream6,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream6_IRQn,
        .channel = DMA_CHANNEL_4,
        .priority = 2,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA1_Stream5,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream5_IRQn,
        .channel = DMA_CHANNEL_4,
        .priority = 1,
        .enable = true,
    },
    .priority = 1,
};

/**************
 * can1 [INTERFACE - CAN]
 **************/
struct up_can_dev_s can1_dev = {
    .dev = {
        .cd_status = 0,
        .cd_baud = 500000,
        .cd_mode = {
            .bm_loopback = 0,
            .bm_silent = 0,
        },
        .cd_ops = &g_can_master_ops,
        .cd_priv = &can1_dev,
    },
    .id = 1,
    .txpin = { .port = GPIOD, .pin = 1, .alternate = GPIO_AF9_CAN1,},
    .rxpin = { .port = GPIOD, .pin = 0, .alternate = GPIO_AF9_CAN1,},
    .priority = 1,
};

/**************
 * spi1 internal --- L3GD20 GYRO + LSM303D ACCEL+MAG + MPU6000 + MS5611-01A
 **************/
struct up_spi_dev_s spi1_dev = 
{
    .dev = {
        .frequency = 10000000,
        .mode = SPIDEV_MODE0,
        .nbits = 8,
        .ops       = &g_spi_ops,
        .priv      = &spi1_dev,
    },
    .id = 1,
    .ncspin =  { .port = GPIOA, .pin = 0,  .alternate = 0,},
    .sckpin =  { .port = GPIOA, .pin = 5, . alternate = GPIO_AF5_SPI1,},
    .misopin = { .port = GPIOA, .pin = 6,  .alternate = GPIO_AF5_SPI1,},
    .mosipin = { .port = GPIOA, .pin = 7, . alternate = GPIO_AF5_SPI1,},
    .txdma_cfg = {
        .instance = DMA2_Stream3,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream3_IRQn,
        .channel = DMA_CHANNEL_3,
        .priority = 5,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA2_Stream2,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream2_IRQn,
        .channel = DMA_CHANNEL_3,
        .priority = 5,
        .enable = true,
    },
    .priority = 4,
    .dev_cs = {
        [0] = {0x22, GPIOC, 13,}, // DRV_GYR_DEVTYPE_L3GD20  0x22
        [1] = {0x11, GPIOC, 15,}, // DRV_IMU_DEVTYPE_LSM303D  0x11
        [2] = {0x21, GPIOC, 2,},  // DRV_IMU_DEVTYPE_MPU6000  0x21
        [3] = {0x3D, GPIOD, 7,},  // DRV_BARO_DEVTYPE_MS5611  0x3D
    }
};

/**************
 * spi2 internal --- FM25V01A
 **************/
struct up_spi_dev_s spi2_dev = 
{
    .dev = {
        .frequency = 10000000,
        .mode = SPIDEV_MODE0,
        .nbits = 8,
        .ops       = &g_spi_ops,
        .priv      = &spi2_dev,
    },
    .id = 2,
    .ncspin =  { .port = GPIOA, .pin = 0,  .alternate = 0,},
    .sckpin =  { .port = GPIOB, .pin = 13, . alternate = GPIO_AF5_SPI2,},
    .misopin = { .port = GPIOB, .pin = 14,  .alternate = GPIO_AF5_SPI2,},
    .mosipin = { .port = GPIOB, .pin = 15, . alternate = GPIO_AF5_SPI2,},
    .txdma_cfg = {
        .instance = DMA1_Stream4,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream4_IRQn,
        .channel = DMA_CHANNEL_0,
        .priority = 7,
        .enable = false,
    },
    .rxdma_cfg = {
        .instance = DMA1_Stream3,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream3_IRQn,
        .channel = DMA_CHANNEL_0,
        .priority = 7,
        .enable = true,
    },
    .priority = 6,
    .dev_cs = {
        [0] = {0x01, GPIOD, 10,}, // DRV_GYR_DEVTYPE_L3GD20  0x22
    }
};

/**************
 * i2c2 internal --- RGB-TCA62724
 **************/
struct up_i2c_master_s i2c2_dev = 
{
    .dev = {
        .cfg = {
            .address = 0,
            .addrlen = 7,
            .frequency = 400000,
        },
        .ops       = &g_i2c_master_ops,
        .priv      = &i2c2_dev,
    },
	.id = 2,
	.sclpin = { .port = GPIOB, .pin = 10,  .alternate = GPIO_AF4_I2C2,},
    .sdapin = { .port = GPIOB, .pin = 11,  .alternate = GPIO_AF4_I2C2,},
    .priority = 4,
    .priority_error = 5,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
    BOARD_INITPIN(GPIO_nLED_PORT, GPIO_nLED_PIN, 
        IOMODE_OUTPP, IO_NOPULL, IO_SPEEDMAX);
    BOARD_INITPIN(GPIO_VDD_5V_PERIPH_nEN_PORT, GPIO_VDD_5V_PERIPH_nEN_PIN,
        IOMODE_OUTPP, IO_PULLUP, IO_SPEEDMAX);
    BOARD_INITPIN(GPIO_VDD_3V3_SENSORS_nEN_PORT, GPIO_VDD_3V3_SENSORS_nEN_PIN,
        IOMODE_OUTPP, IO_PULLDOWN, IO_SPEEDMAX);

    BOARD_INITPIN(GPIO_VDD_5V_SENS_OC_PORT, GPIO_VDD_5V_SENS_OC_PIN,
        IOMODE_INPUT, IO_NOPULL, IO_SPEEDMAX);
    BOARD_INITPIN(GPIO_VDD_5V_HIPOWER_OC_PORT, GPIO_VDD_5V_HIPOWER_OC_PIN,
        IOMODE_INPUT, IO_PULLUP, IO_SPEEDMAX);
    BOARD_INITPIN(GPIO_VDD_5V_PERIPH_OC_PORT, GPIO_VDD_5V_PERIPH_OC_PIN,
        IOMODE_INPUT, IO_PULLUP, IO_SPEEDMAX);

    BOARD_LED(false);
    VDD_5V_PERIPH_EN(true);
    VDD_3V3_SENSOR_EN(true);

    /* delay after sensor power enable */
    HAL_Delay(300); 

    // init spi soft cs pin
    LOW_INITPIN(GPIOC, 2,  IOMODE_OUTPP, IO_PULLUP, IO_SPEEDMAX);
    LOW_INITPIN(GPIOC, 13, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDMAX);
    LOW_INITPIN(GPIOC, 15, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDMAX);
    LOW_INITPIN(GPIOD, 7,  IOMODE_OUTPP, IO_PULLUP, IO_SPEEDMAX);
    LOW_INITPIN(GPIOD, 10,  IOMODE_OUTPP, IO_PULLUP, IO_SPEEDMAX);
    LOW_IOSET(GPIOC, 2, 1);
    LOW_IOSET(GPIOC, 13, 1);
    LOW_IOSET(GPIOC, 15, 1);
    LOW_IOSET(GPIOD, 7, 1);
    LOW_IOSET(GPIOD, 10, 1);

    serial_register(&com2_dev.dev, 2);
    spi_register(&spi1_dev.dev, 1);
    spi_register(&spi2_dev.dev, 2);
    i2c_register(&i2c2_dev.dev, 2);

    serial_bus_initialize(2);
    spi_bus_initialize(1);
    spi_bus_initialize(2);
    i2c_bus_initialize(2);

    dstdout = serial_bus_get(2);
    dstdin = serial_bus_get(2);

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif

    hw_stm32_mmcsd_init(1, 1, 4);
    hw_stm32_mmcsd_info(1);
    hw_stm32_mmcsd_fs_init(1);
}

void board_debug()
{
    board_led_toggle(0);
}

void board_led_toggle(uint8_t idx)
{
    (void)idx;

    int val = LOW_IOGET(GPIO_nLED_PORT, GPIO_nLED_PIN);
    LOW_IOSET(GPIO_nLED_PORT, GPIO_nLED_PIN, !val);
}


#ifdef CONFIG_BOARD_COM_STDINOUT
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
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
size_t fwrite(const void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _write(stream->_file, ptr, size*n_items);
}
size_t fread(void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _read(stream->_file, ptr, size*n_items);
}
#endif



#ifdef CONFIG_FR_IDLE_TIMER_TASKCREATE_HANDLE
#include "FreeRTOS.h"
#include "task.h"
StackType_t xTaskIdle_stack[configMINIMAL_STACK_SIZE];
StaticTask_t xTaskIdle;

StackType_t xTasktimer_stack[configTIMER_TASK_STACK_DEPTH];
StaticTask_t xTasktimer;

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
    StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xTaskIdle;
    *ppxIdleTaskStackBuffer = xTaskIdle_stack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, 
    StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTasktimer;
    *ppxTimerTaskStackBuffer = xTasktimer_stack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif

#ifdef CONFIG_FR_MALLOC_FAILED_HANDLE
#include "FreeRTOS.h"
#include "task.h"
void vApplicationMallocFailedHook( void )
{
    // printf("[ERROR] memory allocate failed, free: %d bytes\r\n", xPortGetFreeHeapSize());
}
#endif

