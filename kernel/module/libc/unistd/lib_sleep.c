#include "unistd.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#elif defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>
#endif

#if defined(CONFIG_RTTNANO_ENABLE)
void msleep(unsigned int msecs)
{
    rt_thread_mdelay(msecs);
}
void ssleep(unsigned int seconds)
{
    msleep(seconds * 1000);
}
void mdelay(unsigned long msecs)
{
    rt_hw_us_delay(msecs * 1000);
}
void udelay(unsigned long usecs)
{
    rt_hw_us_delay(usecs);
}
void ndelay(unsigned long nsecs)
{
    rt_hw_us_delay(1);
}
#endif

unsigned int sleep(unsigned int seconds)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
#elif defined(CONFIG_RTTNANO_ENABLE)
    if (rt_thread_self() != RT_NULL) {
        ssleep(seconds);
    } else {
        while (seconds > 0) {
            udelay(1000000u);
            seconds--;
        }
    }
#endif
    return 0;
}

/**
 * usec - The number of microseconds(1/1000000 s) to sleep.
 */
int usleep(useconds_t usec)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    vTaskDelay(pdMS_TO_TICKS(usec / 1000 + (usec % 1000 != 0)));
#elif defined(CONFIG_RTTNANO_ENABLE)
    if (rt_thread_self() != RT_NULL) {
        msleep(usec / 1000u);
        udelay(usec % 1000u);
    } else {
        udelay(usec);
    }
#endif
    return 0;
}

