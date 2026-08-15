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

int sched_lock()
{
#if defined(CONFIG_FREERTOS_ENABLE)
    vTaskSuspendAll();
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_enter_critical();
#endif

    return 0; // OK
}
