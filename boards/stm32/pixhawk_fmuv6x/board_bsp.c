#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_spi.h>
#include <drv_mmcsd.h>
#include <device/dnode.h>
#include <device/serial.h>
#include <device/i2c_master.h>
#include <device/spi.h>

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

static SemaphoreHandle_t board_printf_sem;

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

	VDD_5V_PERIPH_EN(true);
	VDD_5V_HIPOWER_EN(true);
    VDD_3V3_SENSORS_EN(true);
    VDD_3V3_SPEKTRUM_POWER_EN(true);
    VDD_3V3_SD_CARD_EN(true);
    VDD_3V3_ETH_POWER_EN(true);

    /* delay after sensor power enable */
    HAL_Delay(200); 

	BOARD_BLUE_LED(false);
	BOARD_RED_LED(false);
    BOARD_GREEN_LED(false);

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

#if defined(CONFIG_FREERTOS_ENABLE)
    board_printf_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(board_printf_sem);
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
    case 0: break; //return SERIAL_RDBUF(&com2_dev.dev, p, size);
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
        vTaskDelay(100);
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

