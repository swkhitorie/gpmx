#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <drv_mmcsd.h>
#include <device/dnode.h>

#include "fm25vxx_flash.h"
#include "l3gd20_drv.h"
#include "lsm303d_drv.h"
#include "tca62724.h"

#include <stdarg.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#endif

#if defined(CONFIG_MODULE_CMBACKTRACE)
#include "cm_backtrace.h"
#endif

#if defined(CONFIG_MODULE_KPRINTF)
#include "kprintf.h"
#endif

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
#include "board_usb_cdc.h"
#endif

/** 
 * Pixhawk FMU v2 BSP
 * 
 * I2C2 Internal -> TCA62724(RGB)
 * SPI1 Internal -> L3GD20(CS - PC13, DRDY - PB0) 
 *                  MPU6000(CS - PC2, DRDY - PD15)
 *                  MS5611(CS - PD7)
 *                  LSM303D(CS - PC15, DRDY(accel) - PB4, DRDY(mag) - PB1)
 * SPI2 Internal -> FM25V01(CS)
 */

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
        {0x22, GPIOC, 13,}, // DRV_GYR_DEVTYPE_L3GD20  0x22
        {0x11, GPIOC, 15,}, // DRV_IMU_DEVTYPE_LSM303D  0x11
        {0x21, GPIOC, 2,},  // DRV_IMU_DEVTYPE_MPU6000  0x21
        {0x3D, GPIOD, 7,},  // DRV_BARO_DEVTYPE_MS5611  0x3D
    }
};

/**************
 * spi2 internal --- FM25V01A
 **************/
struct up_spi_dev_s spi2_dev = 
{
    .dev = {
        .frequency = 1000000,
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
        .enable = true,
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
        {0x02, GPIOD, 10,},
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

static SemaphoreHandle_t board_printf_sem;

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

    hw_stm32_rtc_setup();

#if defined(CONFIG_FATFS_ENABLE)
    hw_stm32_mmcsd_init(1, 1, 4);
    hw_stm32_mmcsd_info(1);
    hw_stm32_mmcsd_fs_init(1);
#endif

#if defined(CONFIG_MODULE_CMBACKTRACE)
    cm_backtrace_init("fmuv2_test", "v1.0.0", "v1.0.1");
#endif

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    board_cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
    while(!usb_device_is_configured(0))
    {
#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(100);
#endif
    }
    board_delay(400);
#endif

    board_printf_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(board_printf_sem);
}

void board_bsp_deinit()
{

}

void board_led_toggle(uint8_t idx)
{
    (void)idx;

    int val = LOW_IOGET(GPIO_nLED_PORT, GPIO_nLED_PIN);
    LOW_IOSET(GPIO_nLED_PORT, GPIO_nLED_PIN, !val);
}

rclk_time_t board_rtc_get_timestamp(struct rclk_timeval *now)
{
    return hw_stm32_rtc_get_timeval(now);
}

bool board_rtc_set_timestamp(rclk_time_t now)
{
    return hw_stm32_rtc_set_time_stamp(now);
}

/**
    uint8_t mavlink_reboot_id1[41] = {
        0xfe, 0x21, 0x72, 0xff, 0x00, 0x4c, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xf6, 0x00, 0x01, 0x00, 0x00, 0x53, 0x6b,
    };
    uint8_t mavlink_reboot_id0[41] = {
        0xfe, 0x21, 0x45, 0xff, 0x00, 0x4c, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xf6, 0x00, 0x00, 0x00, 0x00, 0xcc, 0x37,
    };
 */
void upgrade_task(void *p)
{
    uint32_t lst_wakeup = xTaskGetTickCount();
    uint8_t rbuff[128] = {0};
    uint8_t sz = 0;
    uint8_t data[2] = {0x12, 0x10};

    for (;;) {
        vTaskDelayUntil(&lst_wakeup, pdMS_TO_TICKS(10));
        sz = board_cdc_acm_read(0, rbuff, 128);
        if (sz > 35) {
            for (int i = 0; i < sz; i++) {
                if (rbuff[i] == 0xfe) {
                    if ((rbuff[i+1] == 0x21) &&
                        (rbuff[i+2] == 0x45 || rbuff[i+2] == 0x72) &&
                        (rbuff[i+3] == 0xff) &&
                        (rbuff[i+4] == 0x00) &&
                        (rbuff[i+5] == 0x4c))
                    {
                        board_cdc_acm_send(0, data, 2, 0);

                        board_delay(200);

                        board_reboot();
                    }
                }
            }
        }
    }
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
#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    case 1: return board_cdc_acm_read(0, p, size);
#endif
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
        break;
    case 1: {
#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
            return board_cdc_acm_send(0, p, size, 0);
#endif
        }
        break;
    }
    return 0;
}

void board_stream_printf(int port, const char *format, ...)
{
    int idx;
    int iport = port;
    va_list args;

#if defined(CONFIG_MODULE_KPRINTF)
    va_start(args, format);
    vfctprintf_(board_stream_outc, &iport, format, args);
#else
    static char tmp_buffer[512];
    va_start(args, format);
    idx = vsnprintf(tmp_buffer, 512, format, args);
    if (idx > 512) {
        idx = 512;
    }
    board_stream_out(port, tmp_buffer, idx, 0);
#endif

    va_end(args);
}

/****************************************************************************
 * Board printf setting
 ****************************************************************************/
#if defined(CONFIG_MODULE_KPRINTF)
void _putchar(char ch)
{
#if (CONFIG_BOARD_PRINTF_SOURCE == 1)

    SERIAL_SEND(&com2_dev.dev, (const uint8_t *)&ch, 1);
#elif (CONFIG_BOARD_PRINTF_SOURCE == 2)

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    board_cdc_acm_send(0, &ch, 1, 0);
#endif
#endif
}

#else
#include <string.h>
#include <stdio.h>

#if defined (__CC_ARM) || defined(__ARMCC_VERSION)

int fputc(int c, FILE *f)
{
#if (CONFIG_BOARD_PRINTF_SOURCE == 1)
    SERIAL_SEND(&com2_dev.dev, ptr, len);
#elif (CONFIG_BOARD_PRINTF_SOURCE == 2)
#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    board_cdc_acm_send(0, &ch, 1, 0);
#endif
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
        SERIAL_SEND(&com2_dev.dev, ptr, len);
#elif (CONFIG_BOARD_PRINTF_SOURCE == 2)
#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
        board_cdc_acm_send(0, ptr, len, 0);
#endif
#endif
    }

    return len;
}
#endif

#endif // end with CONFIG_MODULE_KPRINTF

void board_printf(const char *format, ...)
{
    va_list args;

#if defined(CONFIG_FREERTOS_ENABLE)
    if (xSemaphoreTake(board_printf_sem, 200) != pdTRUE) {
        return;
    }
#endif

    va_start(args, format);

#if defined(CONFIG_MODULE_KPRINTF)
    vprintf_(format, args);
#else
    vprintf(format, args);
#endif

    va_end(args);

#if defined(CONFIG_FREERTOS_ENABLE)
    xSemaphoreGive(board_printf_sem);
#endif
}

#ifdef CONFIG_FREERTOS_ENABLE

#if (configSUPPORT_STATIC_ALLOCATION == 1)
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

void board_bsp_kernel_init(void *p)
{
    board_bsp_init();

    fm25_flash_init(2);
    l3gd20_drv_init(1);
    lsm303d_drv_init(1);
    rgb_tca62724_init(2);

    extern void main_root(void *p);
    xTaskCreate(main_root, "b2_init", 256, NULL, 3, NULL);
    xTaskCreate(upgrade_task, "upg_detect", 256, NULL, 8, NULL);
    vTaskDelete(NULL);
}

int main(int argc, char *argv[])
{
    taskENTER_CRITICAL(); 
    xTaskCreate(board_bsp_kernel_init, "b1_init", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
#endif

/****************************************************************************
 * TEST
 ****************************************************************************/
#ifdef CONFIG_FREERTOS_ENABLE
#include "freertos_test.h"
#ifdef CONFIG_MODULE_KPOSIX
#include "kernel_libc_tests.h"
#endif
#endif

#if defined(CONFIG_LFS_ENABLE)
#include "lfs_test.h"
#endif

#if defined(CONFIG_FATFS_ENABLE)
#include "fatfs_test.h"
#endif

#if defined(CONFIG_MODULE_CMBACKTRACE)
#include "fault_test.h"
#endif

#ifndef BOARD_TEST_ITEM
#define BOARD_TEST_ITEM board_test_os_start
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
void board_heartbeat_os(void *p)
{
    uint32_t ticks = 0;
    uint8_t rgb_toggle = 0;

    while (1) {
        board_led_toggle(0);

        l3gd20_test();
        lsm303d_test_accel();
        if ((ticks % 10) == 0) {
            rgb_toggle = !rgb_toggle;
            if (rgb_toggle == 0) {
                rgb_tca62724_set(RGB_COLOR_PURPLE, 50);
            } else {
                rgb_tca62724_set(RGB_COLOR_NONE, 0);
            }
        }

        ticks++;
        vTaskDelay(10);
    }
}

void board_test_os(void *p)
{
    while (1) {
        board_printf("test rtos\r\n");

        vTaskDelay(100);
    }
}

int board_test_os_start(int argc, char **argv)
{
    xTaskCreate(board_test_os, "test", 128, NULL, 3, NULL);
    return 0;
}
#endif

void board_test()
{
#ifndef CONFIG_FREERTOS_ENABLE
    uint32_t m1 = board_get_time();

    while (1) {
        if (board_elapsed_time(m1) >= 100) {
            m1 = board_get_time();

            board_led_toggle(0);
            board_printf("test\r\n");
        }
    }
#else
    int ret = BOARD_TEST_ITEM(0, NULL);

    xTaskCreate(board_heartbeat_os, "heartbeat", 256, NULL, 3, NULL);

    vTaskDelete(NULL);
#endif
}
