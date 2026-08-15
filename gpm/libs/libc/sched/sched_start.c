#include "sched.h"
#include "errno.h"
#include <gpmx/config.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#elif defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>
#endif

void sched_start()
{
#if defined(CONFIG_FREERTOS_ENABLE)
    vTaskStartScheduler();
#endif
}
