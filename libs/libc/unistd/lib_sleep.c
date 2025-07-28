#include "unistd.h"

/* FreeRTOS interface include */
#include <FreeRTOS.h>
#include <task.h>

unsigned sleep(unsigned ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
    return 0;
}
