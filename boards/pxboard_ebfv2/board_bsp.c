#include <gpmx/config.h>
#include <board_config.h>
#include <drv_perf.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <drv_rtc.h>
#include <drv_can.h>
#include <drv_eth.h>
#include <drv_mmcsd.h>
#include <gpm/serial/serial.h>
#include <gpm/i2c/i2c_master.h>
#include <gpm/spi/spi.h>
#include <gpm/can/can.h>

#include "driver/drv_sched.h"

#include <stdarg.h>
#include <string.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#if configAPPLICATION_ALLOCATED_HEAP
// CCM RAM Can not run code
__attribute__((section(".ccmram"))) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif
#endif

#if defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>
#include "shell.h"
#endif

#if defined(CONFIG_CMBACKTRACE)
#include "cm_backtrace.h"
#include "hardfault_log.h"
#endif

#if defined(CONFIG_KPRINTF)
#include "kprintf.h"
#endif

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
#include "board_usb_cdc.h"
#endif

#if defined(CONFIG_DRIVER_MMCSDSPI)
#include <gpm/spi/mmcsd_spi.h>
mmcsd_obj_t _board_mmcsd_spi_obj;
#endif

#if defined(CONFIG_HRT)
#include "driver/drv_hrt.h"
#endif

#if defined(CONFIG_GMSH)
#include "gmsh.h"
#include "shell.h"
#endif

#if defined(CONFIG_FS_FATFS)
#include "ff.h"
#endif

#if defined(CONFIG_LIBC_PTHREAD)
#include "pthread.h"
#endif

#if defined(CONFIG_LIBC_VFS)
#include <gpm/fs/fs.h>
#include <fcntl.h>
#endif

/**************
 * uart1 port -- debug/log
 **************/
#if !defined(SERIAL1_CONFIG)
#define SERIAL1_DMA_RXBUFFER_LEN    (128)
#define SERIAL1_DMA_TXBUFFER_LEN    (64)
#define SERIAL1_TXBUFFER_LEN        (64)
#define SERIAL1_RXBUFFER_LEN        (64)
#define SERIAL1_BAUDRATE            (460800)
#endif
uint8_t com1_dma_rxbuff[SERIAL1_DMA_RXBUFFER_LEN];
uint8_t com1_dma_txbuff[SERIAL1_DMA_TXBUFFER_LEN];
uint8_t com1_txbuff[SERIAL1_TXBUFFER_LEN];
uint8_t com1_rxbuff[SERIAL1_RXBUFFER_LEN];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = SERIAL1_BAUDRATE,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = SERIAL1_RXBUFFER_LEN,
            .buffer = com1_rxbuff,
        },
        .xmit = {
            .capacity = SERIAL1_TXBUFFER_LEN,
            .buffer = com1_txbuff,
        },
        .dmarx = {
            .capacity = SERIAL1_DMA_RXBUFFER_LEN,
            .buffer = com1_dma_rxbuff,
        },
        .dmatx = {
            .capacity = SERIAL1_DMA_TXBUFFER_LEN,
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
#if !defined(SERIAL3_CONFIG)
#define SERIAL3_DMA_RXBUFFER_LEN    (128)
#define SERIAL3_DMA_TXBUFFER_LEN    (64)
#define SERIAL3_TXBUFFER_LEN        (64)
#define SERIAL3_RXBUFFER_LEN        (64)
#define SERIAL3_BAUDRATE            (460800)
#endif
uint8_t com3_dma_rxbuff[SERIAL3_DMA_RXBUFFER_LEN];
uint8_t com3_dma_txbuff[SERIAL3_DMA_TXBUFFER_LEN];
uint8_t com3_txbuff[SERIAL3_TXBUFFER_LEN];
uint8_t com3_rxbuff[SERIAL3_RXBUFFER_LEN];
struct up_uart_dev_s com3_dev = {
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = SERIAL3_RXBUFFER_LEN,
            .buffer = com3_rxbuff,
        },
        .xmit = {
            .capacity = SERIAL3_TXBUFFER_LEN,
            .buffer = com3_txbuff,
        },
        .dmarx = {
            .capacity = SERIAL3_DMA_RXBUFFER_LEN,
            .buffer = com3_dma_rxbuff,
        },
        .dmatx = {
            .capacity = SERIAL3_DMA_TXBUFFER_LEN,
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
 * i2c1 Internal --- MPU6050 + AT2402C
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
 * spi2 Internal --- W25Q128JV + W25Q128JV
 **************/
struct up_spi_dev_s spi1_dev = 
{
    .dev = {
        .frequency = 1000000,
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
        {SPIDEV_FLASH(0), GPIOG, 6,},  // DRV_FLASH_DEVTYPE_W25Q  SPIDEV_FLASH
#if defined(CONFIG_DRIVER_MMCSDSPI)
        {SPIDEV_MMCSD(0), GPIOF, 6},
#else
        {0x11, GPIOF, 6,},  // DRV_MODULE_DEVTYPE_USR  0x11
#endif

    }
};

/**************
 * can2 External
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

struct i2c_master_s *g_at24xx_dev = &i2c1_dev.dev;
struct spi_dev_s    *g_w25_dev    = &spi1_dev.dev;

#if defined(CONFIG_FREERTOS_ENABLE)
static SemaphoreHandle_t board_printf_mutex;
#elif defined(CONFIG_RTTNANO_ENABLE)
static rt_sem_t board_printf_mutex;
#endif

extern void main_root(void *p);
#if defined(CONFIG_LIBC_PTHREAD)
extern void* main_proot(void *p);
#endif

void board_bsp_init()
{
    LOW_INITPIN(GPIOG, 6, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOC, 3, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    // usb otg id io
    LOW_INITPIN(GPIOB, 1, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_IOSET(GPIOB, 1, 0);

    LOW_INITPIN(GPIOF, 6, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_IOSET(GPIOF, 6, 1);

    /** PHY Reset Pin Config */
    LOW_INITPIN(GPIOI, 1, IOMODE_OUTPP, IO_PULLUP, IO_SPEEDHIGH);

    serial_register(&com1_dev.dev, 1);
    serial_register(&com3_dev.dev, 3);

    i2c_register(&i2c1_dev.dev, 1);
    spi_register(&spi1_dev.dev, 1);
    can_register(&can2_dev.dev, 2);

    serial_bus_initialize(1);

#if defined(CONFIG_LIBC_VFS)
    uart_register("/dev/ttyS1", &com1_dev.dev);
#endif

    serial_bus_initialize(3);

    i2c_bus_initialize(1);
    spi_bus_initialize(1);
    can_bus_initialize(2);

    hw_stm32_rtc_setup();

#if defined(CONFIG_CMBACKTRACE)
    cm_backtrace_init("pxboard_ebfv2_firmware", "v1.0.0", "v1.0.1");
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
    board_printf_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(board_printf_mutex);
#elif defined(CONFIG_RTTNANO_ENABLE)
    board_printf_mutex = rt_sem_create("blog", 1, RT_IPC_FLAG_PRIO);
#endif

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    board_cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
    while(!usb_device_is_configured(0))
    {
#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(1);
#elif defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_delay(1);
#endif
    }
    board_delay(400);
#endif

#if defined(CONFIG_FS_FATFS) && !defined(CONFIG_DRIVER_MMCSDSPI)
    hw_stm32_mmcsd_init(1, 0, 4, true, true);
    hw_stm32_mmcsd_info(1);
    hw_stm32_mmcsd_fs_init(1);
#endif

#if defined(CONFIG_DRIVER_MMCSDSPI)
    int ret = mmcsd_spi_init(&_board_mmcsd_spi_obj, &spi1_dev.dev, 0);
    if (ret == SM_STATE_READY) {
        hw_mmcsd_spi_fs_init(0);
    };
#endif

#if defined(CONFIG_NET_LWIP_ENABLE)
    hw_stm32_eth_init();
#endif

#if defined(CONFIG_CMBACKTRACE)
    // char *hardfault_log = hardfault_log_check();
    // if (hardfault_log!=NULL) {
    //     board_printf("hardfault_log: %s", hardfault_log);
    //     hardfault_log_clear();
    //     board_delay(5000);
    // }
#endif
}

void board_led_toggle(uint8_t idx)
{
    int val;
    (void)idx;

    val = LOW_IOGET(GPIOC, 3);
    LOW_IOSET(GPIOC, 3, !val);
}

bool board_rtc_set_timestamp(rtc_time_t now)
{
    return hw_stm32_rtc_set_time_stamp(now);
}

rtc_time_t board_rtc_get_timestamp(struct rtc_timeval *now)
{
    return hw_stm32_rtc_get_timeval(now);
}

int board_gpiosetevent(uint32_t pinid, bool risingedge, bool fallingedge,
    bool event, io_irq_entry func, void *arg, uint32_t priority)
{
    uint32_t pinset = 0;
    switch (pinid) {
    case 0x36:
        pinset = GET_PIN(GPIOF, 10);
        break;
    }

    return stm32_pin_setevent(pinset, risingedge, fallingedge, event, func, arg, priority);
}
/****************************************************************************
 * Board Stream serial/usb interface
 ****************************************************************************/
void board_stream_outc(char character, void* arg)
{
    int port = *(int *)arg;
    board_stream_out(port, &character, 1, 0);
}

int board_stream_in(int port, void *p, int size, int way)
{
    switch (port) {
    case 0: return SERIAL_RDBUF(&com1_dev.dev, p, size);
    case 2: return SERIAL_RDBUF(&com3_dev.dev, p, size);
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
    }
    return 0;
}

void board_stream_printf(int port, const char *format, ...)
{
    int idx;
    int iport = port;
    va_list args;

#if defined(CONFIG_KPRINTF)
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
#if defined(CONFIG_KPRINTF)
void _putchar(char ch)
{
    if (ch == '\n') {
        _putchar('\r');
    }

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
    if (ch == '\n') {
        fputc('\r', f);
    }

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

#endif // end with CONFIG_KPRINTF

void board_vprintf(const char *format, va_list args)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    if (xSemaphoreTake(board_printf_mutex, portMAX_DELAY) != pdTRUE) {
        return;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)
    if (rt_sem_take(board_printf_mutex, UINT32_MAX) != RT_EOK) {
        return;
    }
#endif

#if defined(CONFIG_KPRINTF)
    vprintf_(format, args);
#else
    (void)vprintf(format, args);
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
    xSemaphoreGive(board_printf_mutex);
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_sem_release(board_printf_mutex);
#endif
}

void board_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    board_vprintf(format, args);
    va_end(args);
}

#if defined(CONFIG_FREERTOS_ENABLE)
/****************************************************************************
 * FreeRTOS
 ****************************************************************************/

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

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    board_printf("task stack overflow:%s \r\n", pcTaskName);
}
#endif

#if defined(CONFIG_GMSH)
#include "shell.h"
char gmsh_console_getchar()
{
    char c = (char)(-1);
    int ret = board_stream_in(0, &c, 1, 0);

    if (ret != 1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        return -1;
    }

    return c;
}

int gsh_kprintf(const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    board_vprintf(format, args);
    va_end(args);
    return 0;
}
#endif

void board_bsp_kernel_init(void *p)
{
    board_bsp_init();
#if defined(CONFIG_HRT) && (configGENERATE_RUN_TIME_STATS == 0)
    hrt_init();
#endif

#if defined(CONFIG_GMSH)
    gmsh_system_init();
#endif

#if defined(CONFIG_LIBC_PTHREAD)
    pthread_attr_t attr;
    pthread_t id;
    pthread_attr_init(&attr);

    struct sched_param attr_param;
    attr_param.sched_priority = 31;
    pthread_attr_setschedparam(&attr, &attr_param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 4096);
    pthread_create(&id, &attr, main_proot, NULL);
#else
    xTaskCreate(main_root, "u_init", 4096, NULL, 31, NULL);
    vTaskDelete(NULL);
#endif
}

#if defined(CONFIG_LIBC_PTHREAD)
void *main_proot(void *p)
{
#if defined(CONFIG_LIBC_VFS)
    int fd = open("/dev/ttyS1", O_RDOK | O_WROK | O_CREAT);
    if (fd >= 0) {
        dup2(fd, 0);   // copy stdin
        dup2(fd, 1);   // copy stdout
        dup2(fd, 2);   // copy stderr
        if (fd > 2) {
            close(fd);
        }
    }
#endif
    main_root(p);
    return NULL;
}
#endif

int main(int argc, char *argv[])
{
#if defined(CONFIG_LIBC_VFS)
    fs_initialize();
#endif

    taskENTER_CRITICAL(); 
    xTaskCreate(board_bsp_kernel_init, "init", 2048, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}

#elif defined(CONFIG_RTTNANO_ENABLE)
/****************************************************************************
 * RT-Thread Nano
 ****************************************************************************/
#define RTT_NANO_HEAP_SIZE 64*1024
static uint8_t rt_heap[RTT_NANO_HEAP_SIZE];
void *rt_heap_begin_get(void)
{
    return rt_heap;
}
void *rt_heap_end_get(void)
{
    return rt_heap + RTT_NANO_HEAP_SIZE;
}
void rt_hw_us_delay(rt_uint32_t us)
{
    dwt_udelay(us);
}
#if defined(CONFIG_KPRINTF)
int rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args)
{
    return vsnprintf_(buf, size, fmt, args);
}
#endif

#if defined(CONFIG_FINSH)
char rt_hw_console_getchar()
{
    int ch = -1;
    int ret = board_stream_in(0, (char *)&ch, 1, 0);
    if (ret != 1) {
        rt_thread_mdelay(1);
        return ch;
    }

    return ch;
}
#endif
void (*rt_console_output_ptr)(const char *str);
void null_console_output(const char *str)
{
    (void)str;
}
void board_console_output(const char *str)
{
    char ch = 0;
    rt_enter_critical();
    board_printf("%s",str);
    // while (*str!='\0') {
    //     ch = *str++;
    //     _putchar(ch);
    //     // board_stream_out(0, &ch, 1, 0);
	// }
    rt_exit_critical();
}
void rt_hw_console_output(const char *str)
{
    (*rt_console_output_ptr)(str);
}

void rt_hw_board_init(void)
{
#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

    board_init();
#if defined(RT_USING_CPU_USAGE)
    hrt_init();
#endif

    rt_console_output_ptr = null_console_output;
}

#if defined(CONFIG_FS_FATFS) && !defined(CONFIG_DRIVER_MMCSDSPI) && \
    !defined(CONFIG_LIBC_VFS) && !defined(CONFIG_CRUSB_DEVICE_MSC_ENABLE)
static FATFS              mmcsd_fatfs;
static BYTE               mmcsd_work[4096];
#endif

void board_bsp_kernel_init(void *p)
{
    board_bsp_init();

#if defined(CONFIG_HRT) && !defined(RT_USING_CPU_USAGE)
    hrt_init();
#endif

    rt_show_version();

#if defined(CONFIG_FS_FATFS) && !defined(CONFIG_DRIVER_MMCSDSPI) && \
    !defined(CONFIG_LIBC_VFS) && !defined(CONFIG_CRUSB_DEVICE_MSC_ENABLE)
    MKFS_PARM opt;
    opt.fmt = FM_FAT32;
    opt.n_fat = 1;
    opt.align = 0;
    opt.au_size = 0;
    opt.n_root = 0;

    FRESULT ret_ff = f_mount(&mmcsd_fatfs, "0:/", 1);
    if (ret_ff != FR_OK) {
        board_printf("[fat] mmcsd mount failed %d\r\n", ret_ff);
        if (ret_ff == FR_NO_FILESYSTEM) {
            board_printf("[fat] try to format...\r\n");
            ret_ff = f_mkfs("0:/", &opt, mmcsd_work, 4096);
            if (ret_ff != FR_OK) {
                board_printf("[fat] try to format failed %d\r\n", ret_ff);
                return;
            }
            board_printf("[fat] format success\r\n");
            ret_ff = f_mount(&mmcsd_fatfs, "0:/", 1);
            if (ret_ff != FR_OK) {
                board_printf("[fat] mmcsd mount failed after format %d\r\n", ret_ff);
                return;
            }
        } else {
            return;
        }
    }

    f_mount(NULL, "0:/", 1);
    f_mount(&mmcsd_fatfs, "0:/", 0);
#endif

#if defined(CONFIG_LIBC_PTHREAD)
    pthread_attr_t attr;
    pthread_t id;
    pthread_attr_init(&attr);

    struct sched_param attr_param;
    attr_param.sched_priority = 31;
    pthread_attr_setschedparam(&attr, &attr_param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 2048);
    pthread_create(&id, &attr, main_proot, NULL);
#else
    rt_thread_t tid = rt_thread_create("u_init", main_root, NULL, 2048, 0, 2);
    rt_thread_startup(tid);
#endif
}

#if defined(CONFIG_LIBC_PTHREAD)
void *main_proot(void *p)
{
#if defined(CONFIG_LIBC_VFS)
    int fd = open("/dev/ttyS1", O_RDOK | O_WROK | O_CREAT);
    if (fd >= 0) {
        dup2(fd, 0);   // copy stdin
        dup2(fd, 1);   // copy stdout
        dup2(fd, 2);   // copy stderr
        if (fd > 2) {
            close(fd);
        }
    }
#endif
    main_root(p);
    return NULL;
}
#endif

int main()
{
    rt_console_output_ptr = board_console_output;

#if defined(CONFIG_LIBC_VFS)
    fs_initialize();
#endif

    rt_thread_t tid = rt_thread_create("init", board_bsp_kernel_init, NULL, 2048, 3, 2);
    rt_thread_startup(tid);
}
#endif

/****************************************************************************
 * TEST
 ****************************************************************************/
#ifndef CONFIG_TEST_ENTRY
#define CONFIG_TEST_ENTRY null_test_item
#endif

#if defined(CONFIG_FREERTOS_ENABLE) || defined(CONFIG_RTTNANO_ENABLE)
void board_heartbeat_os(void *p)
{

    while (1) {
        board_led_toggle(0);
#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(500);
#elif defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_delay(500);
#endif
    }
}
#endif

int null_test_item(int argc, char **argv) { return 0; }
void board_test()
{
#if !defined(CONFIG_FREERTOS_ENABLE) && !defined(CONFIG_RTTNANO_ENABLE)
    uint32_t m1 = board_get_time();

    while (1) {
        if (board_elapsed_time(m1) >= 100) {
            m1 = board_get_time();

            board_led_toggle(0);
            board_printf("test\r\n");
        }
    }
#elif defined(CONFIG_FREERTOS_ENABLE)
    xTaskCreate(board_heartbeat_os, "heartbeat", 128, NULL, 3, NULL);

    extern int CONFIG_TEST_ENTRY(int argc, char** argv);
    int ret = CONFIG_TEST_ENTRY(0, NULL);

#if !defined(CONFIG_LIBC_PTHREAD)
    vTaskDelete(NULL);
#endif

#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_t hid = rt_thread_create("heartbeat", board_heartbeat_os, NULL, 512, 3, 2);
    rt_thread_startup(hid);

    extern int CONFIG_TEST_ENTRY(int argc, char** argv);
    int ret = CONFIG_TEST_ENTRY(0, NULL);
#endif
}

