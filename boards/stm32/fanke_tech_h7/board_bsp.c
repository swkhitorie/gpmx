#include "board_config.h"
#include <drv_uart.h>
#include <drv_mmcsd.h>
#include <drv_qspi.h>

#include <device/dnode.h>
#include <device/serial.h>
#include <device/qspi.h>

#if defined(CONFIG_STM32_MTD_LFS_SUPPORT)
#include "lfs_sflash_drv.h"
#endif

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
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

#if (CONFIG_BOARD_STDINOUT==1)
uart_dev_t *dstdout;
uart_dev_t *dstdin;
#endif

void board_bsp_init()
{
    LOW_INITPIN(GPIOH, 7, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_IOSET(GPIOH, 7, 1);

    serial_register(&com1_dev.dev, 1);
    serial_bus_initialize(1);

    qspi_register(&quadspi_dev.dev, 1);
    qspi_bus_initialize(1);

#if (CONFIG_BOARD_STDINOUT==1)
    dstdout = serial_bus_get(1);
    dstdin = serial_bus_get(1);
#endif

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
    HAL_Delay(300);
    board_cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
    HAL_Delay(100); // wait cdc init completed
#endif

    hw_stm32_rtc_setup();

#if defined(CONFIG_STM32_MMCSD_FATFS_SUPPORT)
    hw_stm32_mmcsd_init(1, 1, 4);
    hw_stm32_mmcsd_info(1);
    hw_stm32_mmcsd_fs_init(1);
#endif

#if defined(CONFIG_STM32_MTD_LFS_SUPPORT)
    int ret = lfs_sflash_init(1);
    if (ret != 0) {
        printf("lfs sflash init failed: %d\r\n", ret);
    } else {
        printf("lfs sflash success\r\n");
        lfs_sflasg_rwtest();
    }
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

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
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

void board_debug()
{
    board_led_toggle(0);
}

rclk_time_t board_rtc_get_timestamp(struct rclk_timeval *now)
{
    return hw_stm32_rtc_get_timeval(now);
}

bool board_rtc_set_timestamp(rclk_time_t now)
{
    return hw_stm32_rtc_set_time_stamp(now);
}

#if defined(CONFIG_BOARD_STDINOUT) || (CONFIG_BOARD_STDINOUT > 0)
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
int _write(int file, const char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
#if (CONFIG_BOARD_STDINOUT == 1)
        SERIAL_SEND(dstdout, ptr, len);
        // SERIAL_DMASEND(dstdout, ptr, len);
#elif (CONFIG_BOARD_STDINOUT == 2) && defined(CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE)
        board_cdc_acm_send(0, ptr, len, 1);
#endif
    }

    return len;
}

// nonblock
int _read(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    int rsize = 0;
    if (file == stdin_fileno) {
#if (CONFIG_BOARD_STDINOUT == 1)
        rsize = SERIAL_RDBUF(dstdin, ptr, len);
#elif (CONFIG_BOARD_STDINOUT == 2) && defined(CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE)
        rsize = board_cdc_acm_read(0, ptr, len);
#endif
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
    printf("[ERROR] memory allocate failed, free: %d bytes\r\n", xPortGetFreeHeapSize());
}
#endif
