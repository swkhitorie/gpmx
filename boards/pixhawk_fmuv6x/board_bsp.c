#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <drv_rtc.h>
#include <drv_mmcsd.h>
#include <drv_eth.h>
#include "gpm/sched.h"
#include "gpm/serial/serial.h"
#include "gpm/i2c/i2c_master.h"
#include "gpm/spi/spi.h"

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

#if defined(CONFIG_MODULE_CMBACKTRACE)
#include "cm_backtrace.h"
#endif

#if defined(CONFIG_MODULE_KPRINTF)
#include "kprintf.h"
#endif

#if defined(CONFIG_CRUSB_DEVICE_ENABLE) && defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE)
#include "board_usb_cdc.h"
#endif

#if defined(CONFIG_MODULE_HRT)
#include <gpm/drv_hrt.h>
#endif

#if defined(CONFIG_MODULE_GMSH)
#include "gmsh.h"
#include "shell.h"
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
static SemaphoreHandle_t board_printf_mutex;
#elif defined(CONFIG_RTTNANO_ENABLE)
static rt_sem_t board_printf_mutex;
#endif

void board_bsp_init()
{
	LOW_INITPINF(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
    LOW_INITPINF(GPIO_nLED_GREEN_PORT, GPIO_nLED_GREEN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

	// Power in detector
	LOW_INITPINF(GPIO_nPOWER_IN_A_PORT, GPIO_nPOWER_IN_A_PIN, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_nPOWER_IN_B_PORT, GPIO_nPOWER_IN_B_PIN, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_nPOWER_IN_C_PORT, GPIO_nPOWER_IN_C_PIN, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);

	// periph output, hipower ctrl
	LOW_INITPINF(GPIO_VDD_5V_PERIPH_nEN_PORT, GPIO_VDD_5V_PERIPH_nEN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_VDD_5V_PERIPH_nOC_PORT, GPIO_VDD_5V_PERIPH_nOC_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_VDD_5V_HIPOWER_nEN_PORT, GPIO_VDD_5V_HIPOWER_nEN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_VDD_5V_HIPOWER_nOC_PORT, GPIO_VDD_5V_HIPOWER_nOC_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_VDD_3V3_SENSORS_EN_PORT, GPIO_VDD_3V3_SENSORS_EN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_VDD_3V3_SPEKTRUM_POWER_EN_PORT, GPIO_VDD_3V3_SPEKTRUM_POWER_EN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	LOW_INITPINF(GPIO_VDD_3V3_SD_CARD_EN_PORT, GPIO_VDD_3V3_SD_CARD_EN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
    LOW_INITPINF(GPIO_OTGFS_VBUS_PORT, GPIO_OTGFS_VBUS_PIN, GPIO_MODE_INPUT, GPIO_PULLDOWN, GPIO_SPEED_FREQ_VERY_HIGH);
    LOW_INITPINF(GPIO_ETH_POWER_EN_PORT, GPIO_ETH_POWER_EN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);

    VDD_3V3_SD_CARD_EN(true);
	VDD_5V_PERIPH_EN(true);
	VDD_5V_HIPOWER_EN(true);
    VDD_3V3_SENSORS_EN(true);
    VDD_3V3_SPEKTRUM_POWER_EN(true);
    VDD_3V3_ETH_POWER_EN(true);

    /* delay after sensor power enable */
    HAL_Delay(200); 

	BOARD_BLUE_LED(false);
	BOARD_RED_LED(false);
    BOARD_GREEN_LED(false);

#if defined(CONFIG_MODULE_CMBACKTRACE)
    cm_backtrace_init("fmuv6x_firmware", "v1.0.0", "v1.0.1");
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

#if defined(CONFIG_FATFS_ENABLE)
    hw_stm32_mmcsd_init(2, 1, 4, true, true);
    hw_stm32_mmcsd_info(2);
    hw_stm32_mmcsd_fs_init(2);
#endif

#if defined(CONFIG_NET_LWIP_ENABLE)
    hw_stm32_eth_init();
#endif
}

void board_bsp_deinit()
{

}

void board_led_toggle(uint8_t idx)
{
    int val;
    switch (idx) {
    case 0: 
        val = LOW_IOGET(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN);
        LOW_IOSET(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, !val);
        break;
    case 1: 
        val = LOW_IOGET(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN);
        LOW_IOSET(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN, !val);
        break;
    case 2: 
        val = LOW_IOGET(GPIO_nLED_GREEN_PORT, GPIO_nLED_GREEN_PIN);
        LOW_IOSET(GPIO_nLED_GREEN_PORT, GPIO_nLED_GREEN_PIN, !val);
        break;
    }
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

    return 0;
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
    case 0: break; //return SERIAL_RDBUF(&com2_dev.dev, p, size);
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
            // if (way == 0) {
            //     return SERIAL_SEND(&com2_dev.dev, p, size);
            // } else {
            //     return SERIAL_DMASEND(&com2_dev.dev, p, size);
            // }
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
    if (ch == '\n') {
        _putchar('\r');
    }

#if (CONFIG_BOARD_PRINTF_SOURCE == 1)

    // SERIAL_SEND(&com2_dev.dev, (const uint8_t *)&ch, 1);
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
    // SERIAL_SEND(&com2_dev.dev, ptr, len);
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
        // SERIAL_SEND(&com2_dev.dev, ptr, len);
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

#if defined(CONFIG_MODULE_KPRINTF)
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

#ifdef CONFIG_FREERTOS_ENABLE
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

#if defined(CONFIG_MODULE_GMSH)
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
#if defined(CONFIG_MODULE_HRT) && (configGENERATE_RUN_TIME_STATS == 0)
    hrt_init();
#endif
#if defined(CONFIG_MODULE_GMSH)
    gmsh_system_init();
#endif
    extern void main_root(void *p);
    xTaskCreate(main_root, "b2_init", 2048, NULL, 3, NULL);
    vTaskDelete(NULL);
}

int main(int argc, char *argv[])
{
    taskENTER_CRITICAL(); 
    xTaskCreate(board_bsp_kernel_init, "b1_init", 2048, NULL, 3, NULL);
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
#if defined(CONFIG_MODULE_KPRINTF)
int rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args)
{
    return vsnprintf_(buf, size, fmt, args);
}
#endif

#if defined(CONFIG_MODULE_FINSH)
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
    rt_console_output_ptr = null_console_output;
}

void board_bsp_kernel_init(void *p)
{
    board_bsp_init();

#if defined(CONFIG_MODULE_HRT)
    hrt_init();
#endif

    rt_show_version();
    extern void main_root(void *p);
    rt_thread_t tid = rt_thread_create("b2_init", main_root, NULL, 2048, 3, 2);
    rt_thread_startup(tid);
}

int main()
{
    rt_console_output_ptr = board_console_output;
    rt_thread_t tid = rt_thread_create("b1_init", board_bsp_kernel_init, NULL, 2048, 3, 2);
    rt_thread_startup(tid);
}
#endif

/****************************************************************************
 * TEST
 ****************************************************************************/
#ifndef BOARD_TEST_ITEM
#define BOARD_TEST_ITEM null_test_item
#endif

#if defined(CONFIG_FREERTOS_ENABLE) || defined(CONFIG_RTTNANO_ENABLE)
void board_heartbeat_os(void *p)
{
    while (1) {
        board_led_toggle(0);
        board_led_toggle(1);
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

    extern int BOARD_TEST_ITEM(int argc, char** argv);
    int ret = BOARD_TEST_ITEM(0, NULL);

    vTaskDelete(NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_t hid = rt_thread_create("heartbeat", board_heartbeat_os, NULL, 512, 3, 2);
    rt_thread_startup(hid);

    extern int BOARD_TEST_ITEM(int argc, char** argv);
    int ret = BOARD_TEST_ITEM(0, NULL);
#endif
}

