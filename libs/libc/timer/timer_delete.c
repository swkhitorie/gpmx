#include <stddef.h>
#include "pthread.h"
#include "errno.h"
#include "time.h"
#include "utils.h"

#include "./prv_timer.h"

int timer_delete(timer_t timerid)
{
    TimerHandle_t handle = timerid;
    timer_internal_t *p = (timer_internal_t *)pvTimerGetTimerID(handle);

    configASSERT(p != NULL);
    (void)xTimerStop(handle, portMAX_DELAY);

    while (xTimerIsTimerActive(handle) == pdTRUE ) {
        vTaskDelay(1);
    }

    vPortFree(p);
    return 0;
}
