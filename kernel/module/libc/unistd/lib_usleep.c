#include "unistd.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#endif

int usleep(useconds_t usec)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    vTaskDelay(pdMS_TO_TICKS(usec / 1000 + (usec % 1000 != 0)));
#endif
    return 0;
}
