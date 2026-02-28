#include "unistd.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#endif

unsigned sleep(unsigned ms)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    vTaskDelay(pdMS_TO_TICKS(ms));
#endif
    return 0;
}
