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

int sched_yield()
{
#if defined(CONFIG_FREERTOS_ENABLE)
    taskYIELD();
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_yield();
#endif

    return 0;
}
