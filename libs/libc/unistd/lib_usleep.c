#include "unistd.h"

/* FreeRTOS interface include */
#include <FreeRTOS.h>
#include <task.h>

int usleep(useconds_t usec)
{
    vTaskDelay(pdMS_TO_TICKS(usec / 1000 + (usec % 1000 != 0)));
    return 0;
}
