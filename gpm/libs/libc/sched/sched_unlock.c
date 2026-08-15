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

int sched_unlock()
{
#if defined(CONFIG_FREERTOS_ENABLE)
    xTaskResumeAll();
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_exit_critical();
#endif

    return EOPNOTSUPP;
}
