#include "board_config.h"

#include <drv_uart.h>
#include <drv_mmcsd.h>
#include <drv_qspi.h>

#include <device/dnode.h>
#include <device/serial.h>
#include <device/qspi.h>

#include <stdarg.h>

#include "w25qxx_driver.h"

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

/* COM1 */
uint8_t com1_dma_rxbuff[4096];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[256];
uint8_t com1_rxbuff[2048];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 2048,
            .buffer = com1_rxbuff,
        },
        .xmit = {
            .capacity = 256,
            .buffer = com1_txbuff,
        },
        .dmarx = {
            .capacity = 2048,
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
        .instance = DMA1_Stream0,
        .dma_rcc = RCC_AHB1ENR_DMA1EN,
        .dma_irq = DMA1_Stream0_IRQn,
        .request = DMA_REQUEST_USART1_TX,
        .priority = 2,
        .enable = true,
    },
    .rxdma_cfg = {
        .instance = DMA2_Stream0,
        .dma_rcc = RCC_AHB1ENR_DMA2EN,
        .dma_irq = DMA2_Stream0_IRQn,
        .request = DMA_REQUEST_USART1_RX,
        .priority = 1,
        .enable = true,
    },
    .priority = 1,
};

struct up_qspi_dev_s quadspi_dev = 
{
    .dev = {
        .ops       = &g_qspi_ops,
        .priv      = &quadspi_dev,
    },
    .medium_size = 8*1024*1024,
    .memmap = false,
    .ddr_mode = 0,
    .id = 1,
    .ncspin = { .port = GPIOB, .pin = 6,  .alternate = GPIO_AF10_QUADSPI,},
    .sckpin = { .port = GPIOB, .pin = 2,  .alternate = GPIO_AF9_QUADSPI,},
    .io0pin = { .port = GPIOD, .pin = 11, .alternate = GPIO_AF9_QUADSPI,},
    .io1pin = { .port = GPIOD, .pin = 12, .alternate = GPIO_AF9_QUADSPI,},
    .io2pin = { .port = GPIOE, .pin = 2,  .alternate = GPIO_AF9_QUADSPI,},
    .io3pin = { .port = GPIOD, .pin = 13, .alternate = GPIO_AF9_QUADSPI,},
    .mdma_enable = false,
    .mdma_priority = 7,
    .priority = 10,
};

void board_bsp_init()
{
    LOW_INITPIN(GPIOH, 7, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_IOSET(GPIOH, 7, 1);

    serial_register(&com1_dev.dev, 1);
    serial_bus_initialize(1);

    qspi_register(&quadspi_dev.dev, 1);
    qspi_bus_initialize(1);

    hw_stm32_rtc_setup();

#if defined(CONFIG_FATFS_ENABLE)
    hw_stm32_mmcsd_init(1, 1, 4);
    hw_stm32_mmcsd_info(1);
    hw_stm32_mmcsd_fs_init(1);
#endif

#if defined(CONFIG_LFS_ENABLE)
    w25qxx_init(1, true);
#else
    w25qxx_init(1, false);
#endif

#if defined(CONFIG_MODULE_CMBACKTRACE)
    cm_backtrace_init("fankeh7_test", "v1.0.0", "v1.0.1");
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
}

void board_bsp_deinit()
{
    __HAL_RCC_GPIOH_CLK_DISABLE();

    __HAL_RCC_USART1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
    HAL_DMA_DeInit(com1_dev.com.hdmatx);
    HAL_DMA_DeInit(com1_dev.com.hdmarx);
    HAL_NVIC_DisableIRQ(USART1_IRQn);

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    usbd_deinitialize(0);
#endif

    HAL_DeInit();
}

void board_led_toggle(uint8_t idx)
{
    switch (idx) {
    case 0: {
            int val = LOW_IOGET(GPIOH, 7);
            LOW_IOSET(GPIOH, 7, !val);
        }
        break;
    }
}

rclk_time_t board_rtc_get_timestamp(struct rclk_timeval *now)
{
    return hw_stm32_rtc_get_timeval(now);
}

bool board_rtc_set_timestamp(rclk_time_t now)
{
    return hw_stm32_rtc_set_time_stamp(now);
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
                return SERIAL_SEND(&com1_dev.dev, p, size);
            } else {
                return SERIAL_DMASEND(&com1_dev.dev, p, size);
            }
        }
        break;
    case 1: {
#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
            return board_cdc_acm_send(0, p, size, 1);
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

    SERIAL_SEND(&com1_dev.dev, (const uint8_t *)&ch, 1);
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
    SERIAL_SEND(&com1_dev.dev, ptr, len);
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
        SERIAL_SEND(&com1_dev.dev, ptr, len);
#elif (CONFIG_BOARD_PRINTF_SOURCE == 2)
#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
        board_cdc_acm_send(0, &ch, 1, 0);
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
    va_start(args, format);

#if defined(CONFIG_MODULE_KPRINTF)
    vprintf_(format, args);
#else
    vprintf(format, args);
#endif

    va_end(args);
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
    xTaskCreate(main_root, "b2_init", 256, NULL, 3, NULL);
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
    while (1) {
        board_led_toggle(0);
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
    int ret = BOARD_TEST_ITEM(0, NULL);

    xTaskCreate(board_heartbeat_os, "heartbeat", 128, NULL, 3, NULL);

    vTaskDelete(NULL);
#endif
}
