#include "sched.h"
#include "errno.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#elif defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>
#endif

int sched_get_priority_max(int policy)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    (void)policy;
    return configMAX_PRIORITIES - 1;
#elif defined(CONFIG_RTTNANO_ENABLE)
    if (policy != SCHED_FIFO && policy != SCHED_RR)
        return EINVAL;
    return RT_THREAD_PRIORITY_MAX - 1;
#endif
}

int sched_get_priority_min(int policy)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    (void)policy;
    return tskIDLE_PRIORITY;
#elif defined(CONFIG_RTTNANO_ENABLE)
    if (policy != SCHED_FIFO && policy != SCHED_RR)
        return EINVAL;
    return 0;
#endif
}

int sched_yield()
{
#if defined(CONFIG_FREERTOS_ENABLE)
    taskYIELD();
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_yield();
#endif

    return 0;
}

void sched_start()
{
#if defined(CONFIG_FREERTOS_ENABLE)
    vTaskStartScheduler();
#endif
}

int sched_setscheduler(pid_t pid, int policy)
{
    (void)pid;
    (void)policy;
    return EOPNOTSUPP;
}
