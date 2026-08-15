#include <gpmx/config.h>
#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <drv_rtc.h>
#include <drv_mmcsd.h>

#include <gpm/serial/serial.h>
#include <gpm/i2c/i2c_master.h>
#include <gpm/spi/spi.h>

#include "driver/drv_sched.h"

#include "fm25vxx_flash.h"
#include "l3gd20_drv.h"
#include "lsm303d_drv.h"
#include "tca62724.h"

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
#endif

#if defined(CONFIG_KPRINTF)
#include "kprintf.h"
#endif

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
#include "board_usb_cdc.h"
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
        {SPIDEV_FLASH(0), GPIOD, 10,},
    }
};

/**************
 * spi4 external
 **************/
struct up_spi_dev_s spi4_dev = 
{
    .dev = {
        .frequency = 1000000,
        .mode = SPIDEV_MODE0,
        .nbits = 8,
        .ops       = &g_spi_ops,
        .priv      = &spi4_dev,
    },
    .id = 4,
    .ncspin =  { .port = GPIOA, .pin = 0,  .alternate = 0,},
    .sckpin =  { .port = GPIOE, .pin = 2, . alternate = GPIO_AF5_SPI4,},
    .misopin = { .port = GPIOE, .pin = 5,  .alternate = GPIO_AF5_SPI4,},
    .mosipin = { .port = GPIOE, .pin = 6, . alternate = GPIO_AF5_SPI4,},
    .txdma_cfg = {
        .enable = false,
    },
    .rxdma_cfg = {
        .enable = false,
    },
    .priority = 6,
    .dev_cs = {
        {0x22, GPIOC, 13,}, // DRV_GYR_DEVTYPE_L3GD20  0x22
        {0x11, GPIOC, 15,}, // DRV_IMU_DEVTYPE_LSM303D  0x11
        {0x21, GPIOC, 2,},  // DRV_IMU_DEVTYPE_MPU6000  0x21
        {0x3D, GPIOD, 7,},  // DRV_BARO_DEVTYPE_MS5611  0x3D
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

struct spi_dev_s *g_ramtron_dev = &spi2_dev.dev;

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
    VDD_5V_PERIPH_EN(false);
    VDD_3V3_SENSOR_EN(false);

    HAL_Delay(100);

    VDD_5V_PERIPH_EN(true);
    VDD_3V3_SENSOR_EN(true);

    /* delay after sensor power enable */
    HAL_Delay(400); 

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
    spi_register(&spi4_dev.dev, 4);
    i2c_register(&i2c2_dev.dev, 2);

    serial_bus_initialize(2);
    spi_bus_initialize(1);
    spi_bus_initialize(2);
    spi_bus_initialize(4);
    i2c_bus_initialize(2);

    hw_stm32_rtc_setup();

#if defined(CONFIG_CMBACKTRACE)
    cm_backtrace_init("fmuv2_firmware", "v1.0.0", "v1.0.1");
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

#if defined(CONFIG_LIBC_VFS)
    cdc_acm_register("/dev/ttyUSB0", 0);
#endif

#endif

#if defined(CONFIG_FATFS_ENABLE)
    hw_stm32_mmcsd_init(1, 0, 4, true, false);
    hw_stm32_mmcsd_info(1);
    hw_stm32_mmcsd_fs_init(1);
#endif
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
    case 0: return SERIAL_RDBUF(&com2_dev.dev, p, size);
#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
    case 1: return board_cdc_acm_read(0, p, size, way);
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
    if (ch == '\n') {
        fputc('\r', f);
    }

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
    int ret = board_stream_in(1, &c, 1, 0);

    if (ret != 1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        return -1;
    }

    return c;
}

void gsh_kprintf(const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    board_vprintf(format, args);
    va_end(args);
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
    int fd = open("/dev/ttyUSB0", O_RDOK | O_WROK | O_CREAT);
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
    int ret = board_stream_in(1, (char *)&ch, 1, 0);
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
    int fd = open("/dev/ttyUSB0", O_RDOK | O_WROK | O_CREAT);
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

    rt_thread_t tid = rt_thread_create("b1_init", board_bsp_kernel_init, NULL, 2048, 3, 2);
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
    uint32_t ticks = 0;
    uint8_t rgb_toggle = 0;

    // fm25_flash_init(2);
    rgb_tca62724_init(2);
    // fm25_flash_test(0, NULL);
    // mpu6000_drv_init(1);

    while (1) {
        board_led_toggle(0);
        if ((ticks % 2) == 0) {
            rgb_toggle = !rgb_toggle;
            if (rgb_toggle == 0) {
                rgb_tca62724_set(RGB_COLOR_GREEN, 50);
            } else {
                rgb_tca62724_set(RGB_COLOR_NONE, 0);
            }
        }

        ticks++;
#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(100);
#elif defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_delay(100);
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

    vTaskDelete(NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_t hid = rt_thread_create("heartbeat", board_heartbeat_os, NULL, 512, 3, 2);
    rt_thread_startup(hid);

    extern int CONFIG_TEST_ENTRY(int argc, char** argv);
    int ret = CONFIG_TEST_ENTRY(0, NULL);
#endif
}
