#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <drv_rtc.h>
#include <drv_can.h>
#include <drv_eth.h>
#include <device/dnode.h>
#include <device/serial.h>
#include <device/i2c_master.h>
#include <device/spi.h>
#include <device/can.h>

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

/**************
 * uart1 port -- debug/log
 **************/
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 460800,
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
    .id = 1,
    .txpin = { .port = GPIOA, .pin = 9,  .alternate = GPIO_AF7_USART1,},
    .rxpin = { .port = GPIOA, .pin = 10, .alternate = GPIO_AF7_USART1,},
    .txdma_cfg = {
        .instance = DMA2_Stream7,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream7_IRQn,
        .channel = DMA_CHANNEL_4,
        .priority = 5,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA2_Stream2,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream2_IRQn,
        .channel = DMA_CHANNEL_4,
        .priority = 6,
        .enable = true,
    },
    .priority = 4,
};

/**************
 * uart3 port -- esp8266
 **************/
uint8_t com3_dma_rxbuff[4096];
uint8_t com3_dma_txbuff[512];
uint8_t com3_txbuff[512];
uint8_t com3_rxbuff[2048];
struct up_uart_dev_s com3_dev = {
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 2048,
            .buffer = com3_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com3_txbuff,
        },
        .dmarx = {
            .capacity = 4096,
            .buffer = com3_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 512,
            .buffer = com3_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com3_dev,
    },
    .id = 3,
    .txpin = { .port = GPIOB, .pin = 10, .alternate = GPIO_AF7_USART3,},
    .rxpin = { .port = GPIOB, .pin = 11, .alternate = GPIO_AF7_USART3,},
    .txdma_cfg = {
        .instance = DMA1_Stream3,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream3_IRQn,
        .channel = DMA_CHANNEL_4,
        .priority = 2,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA1_Stream1,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream1_IRQn,
        .channel = DMA_CHANNEL_4,
        .priority = 1,
        .enable = true,
    },
    .priority = 1,
};

/**************
 * uart5 port -- for gnss rtcm3 data, Disable for sdmmc
 **************/
uint8_t com5_dma_rxbuff[2048];
uint8_t com5_dma_txbuff[128];
uint8_t com5_txbuff[128];
uint8_t com5_rxbuff[1024];
struct up_uart_dev_s com5_dev = {
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 1024,
            .buffer = com5_rxbuff,
        },
        .xmit = {
            .capacity = 128,
            .buffer = com5_txbuff,
        },
        .dmarx = {
            .capacity = 2048,
            .buffer = com5_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 128,
            .buffer = com5_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com5_dev,
    },
    .id = 5,
    .txpin = { .port = GPIOC, .pin = 12, .alternate = GPIO_AF8_UART5,},
    .rxpin = { .port = GPIOD, .pin = 2,  .alternate = GPIO_AF8_UART5,},
    .txdma_cfg = {
        .enable = false,
    },
    .rxdma_cfg = {
        .instance = DMA1_Stream0,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream0_IRQn,
        .channel = DMA_CHANNEL_4,
        .priority = 1,
        .enable = true,
    },
    .priority = 1,
};

/**************
 * uart6 port -- for gnss rtcm3 data, Disable for camera and speaker interface
 **************/
uint8_t com6_dma_rxbuff[3000];
uint8_t com6_dma_txbuff[512];
uint8_t com6_txbuff[512];
uint8_t com6_rxbuff[1500];
struct up_uart_dev_s com6_dev = {
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 2048,
            .buffer = com6_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com6_txbuff,
        },
        .dmarx = {
            .capacity = 4096,
            .buffer = com6_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 512,
            .buffer = com6_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com6_dev,
    },
    .id = 6,
    .txpin = { .port = GPIOC, .pin = 6, .alternate = GPIO_AF8_USART6,},
    .rxpin = { .port = GPIOC, .pin = 7, .alternate = GPIO_AF8_USART6,},
    .txdma_cfg = {
        .enable = false,
    },
    .rxdma_cfg = {
        .instance = DMA2_Stream1,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream1_IRQn,
        .channel = DMA_CHANNEL_5,
        .priority = 3,
        .enable = true,
    },
    .priority = 2,
};

/**************
 * Internal Sensor I2C --- MPU6050
 **************/
struct up_i2c_master_s i2c1_dev = 
{
    .dev = {
        .cfg = {
            .address = 0,
            .addrlen = 7,
            .frequency = 100000,
        },
        .ops       = &g_i2c_master_ops,
        .priv      = &i2c1_dev,
    },
	.id = 1,
	.sclpin = { .port = GPIOB, .pin = 8,  .alternate = GPIO_AF4_I2C1,},
    .sdapin = { .port = GPIOB, .pin = 9,  .alternate = GPIO_AF4_I2C1,},
    .priority = 4,
    .priority_error = 5,
};

/**************
 * SPI device--- W25Q128JV
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
    .ncspin =  { .port = GPIOB, .pin = 0,  .alternate = 0,},
    .sckpin =  { .port = GPIOB, .pin = 3, . alternate = GPIO_AF5_SPI1,},
    .misopin = { .port = GPIOB, .pin = 4,  .alternate = GPIO_AF5_SPI1,},
    .mosipin = { .port = GPIOB, .pin = 5, . alternate = GPIO_AF5_SPI1,},
    .txdma_cfg = {
        .instance = DMA2_Stream3,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream3_IRQn,
        .channel = DMA_CHANNEL_3,
        .priority = 5,
        .enable = false,
    },
    .rxdma_cfg = {
        .instance = DMA2_Stream2,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream2_IRQn,
        .channel = DMA_CHANNEL_3,
        .priority = 5,
        .enable = false,
    },
    .priority = 4,
    .dev_cs = {
        {0x22, GPIOG, 6,},  // DRV_FLASH_DEVTYPE_W25Q  0x22
        {0x11, GPIOF, 6,},  // DRV_MODULE_DEVTYPE_USR  0x11
    }
};

/**************
 * EXternal CAN Port
 **************/
struct up_can_dev_s can2_dev = {
    .dev = {
        .cd_status = 0,
        .cd_baud = 500000,
        .cd_mode = {
            .bm_loopback = 0,
            .bm_silent = 0,
        },
        .cd_ops = &g_can_master_ops,
        .cd_priv = &can2_dev,
    },
    .id = 2,
	.txpin = { .port = GPIOB, .pin = 13,  .alternate = GPIO_AF9_CAN1,},
    .rxpin = { .port = GPIOB, .pin = 12,  .alternate = GPIO_AF9_CAN1,},
    .priority = 1,
};

#if defined(CONFIG_FREERTOS_ENABLE)
static SemaphoreHandle_t board_printf_mutex;
#endif

void board_bsp_init()
{
    LOW_INITPIN(GPIOG, 6, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOC, 3, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    // usb otg id io
    LOW_INITPIN(GPIOB, 1, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_IOSET(GPIOB, 1, 0);

    serial_register(&com1_dev.dev, 1);
    serial_register(&com3_dev.dev, 3);
    serial_register(&com5_dev.dev, 5);
    serial_register(&com6_dev.dev, 6);

    i2c_register(&i2c1_dev.dev, 1);
    spi_register(&spi1_dev.dev, 1);
    can_register(&can2_dev.dev, 2);

    serial_bus_initialize(1);
    serial_bus_initialize(3);
    serial_bus_initialize(5);
    serial_bus_initialize(6);

    i2c_bus_initialize(1);
    spi_bus_initialize(1);
    can_bus_initialize(2);

    hw_stm32_rtc_setup();

    // hw_stm32_eth_init();

#if defined(CONFIG_MODULE_CMBACKTRACE)
    cm_backtrace_init("pxboard_ebfv2_firmware", "v1.0.0", "v1.0.1");
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
    board_printf_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(board_printf_mutex);
#endif

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    board_cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
    while(!usb_device_is_configured(0))
    {
#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(1);
#endif
    }
    board_delay(400);
#endif

}

void board_led_toggle(uint8_t idx)
{
    int val;
    (void)idx;

    val = LOW_IOGET(GPIOC, 3);
    LOW_IOSET(GPIOC, 3, !val);
}

bool board_rtc_set_timestamp(rclk_time_t now)
{
    return hw_stm32_rtc_set_time_stamp(now);
}

rclk_time_t board_rtc_get_timestamp(struct rclk_timeval *now)
{
    return hw_stm32_rtc_get_timeval(now);
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
    case 0: return SERIAL_RDBUF(&com1_dev.dev, p, size);
    case 2: return SERIAL_RDBUF(&com3_dev.dev, p, size);
    case 4: return SERIAL_RDBUF(&com5_dev.dev, p, size);
    case 5: return SERIAL_RDBUF(&com6_dev.dev, p, size);
    }
    return 0;
}

int board_stream_out(int port, const void *p, int size, int way)
{
    switch (port) {
    case 0: {
            if (way == 0) {
                return SERIAL_SEND(&com1_dev.dev, p, size);
            } else {
                return SERIAL_DMASEND(&com1_dev.dev, p, size);
            }
        }
        break;
    case 2: {
            if (way == 0) {
                return SERIAL_SEND(&com3_dev.dev, p, size);
            } else {
                return SERIAL_DMASEND(&com3_dev.dev, p, size);
            }
        }
        break;
    case 4: {
            if (way == 0) {
                return SERIAL_SEND(&com5_dev.dev, p, size);
            } else {
                return SERIAL_DMASEND(&com5_dev.dev, p, size);
            }
        }
        break;
    case 5: {
            if (way == 0) {
                return SERIAL_SEND(&com6_dev.dev, p, size);
            } else {
                return SERIAL_DMASEND(&com6_dev.dev, p, size);
            }
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

    SERIAL_SEND(&com1_dev.dev, &ch, 1);
#endif
}

#else
#include <string.h>
#include <stdio.h>

#if defined (__CC_ARM) || defined(__ARMCC_VERSION)

int fputc(int c, FILE *f)
{
#if (CONFIG_BOARD_PRINTF_SOURCE == 1)
    SERIAL_SEND(&com1_dev.dev, ptr, len);
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
        SERIAL_SEND(&com1_dev.dev, ptr, len);
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
    if (xSemaphoreTake(board_printf_mutex, portMAX_DELAY) != pdTRUE) {
        return;
    }
#endif

    va_start(args, format);

#if defined(CONFIG_MODULE_KPRINTF)
    vprintf_(format, args);
#else
    (void)vprintf(format, args);
#endif
    va_end(args);

#if defined(CONFIG_FREERTOS_ENABLE)
    xSemaphoreGive(board_printf_mutex);
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
    extern void main_root(void *p);
    xTaskCreate(main_root, "b2_init", 2048, NULL, 3, NULL);
    vTaskDelete(NULL);
}

int main(int argc, char *argv[])
{
    taskENTER_CRITICAL(); 
    xTaskCreate(board_bsp_kernel_init, "b1_init", 512, NULL, 3, NULL);
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
#include "kernel_libc_tests.h"
#ifdef CONFIG_MODULE_HRT
#include "hrt_test.h"
#endif
#endif

#ifndef BOARD_TEST_ITEM
#define BOARD_TEST_ITEM board_test_os_start
#endif

__attribute__((section(".SRAM_region"))) static volatile uint8_t datasr[1024];
#if defined(CONFIG_FREERTOS_ENABLE)
void board_heartbeat_os(void *p)
{
    while (1) {
        #if 0
        for (int i = 0; i < 1024; i++) {
            datasr[i] = i;
        }
        for (int i = 0; i < 100; i++) {
            board_printf("sram val: %x, %x\r\n", &datasr[i], datasr[i]);
        }
        #endif
        board_led_toggle(0);
        board_printf("rtos heartbeat\r\n");
        vTaskDelay(500);
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
    xTaskCreate(board_heartbeat_os, "heartbeat", 128, NULL, 3, NULL);

    int ret = BOARD_TEST_ITEM(0, NULL);

    vTaskDelete(NULL);
#endif
}
