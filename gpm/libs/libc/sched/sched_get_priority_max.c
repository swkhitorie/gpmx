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
    return 0;
}
