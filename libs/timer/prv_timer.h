#ifndef PRV_TIMER_H_
#define PRV_TIMER_H_

#include "signal.h"

/* FreeRTOS interface include */
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

typedef struct timer_internal {
    StaticTimer_t buff;
    struct sigevent event;
    TickType_t period;
} timer_internal_t;

void prv_timer_callback(TimerHandle_t handle);

#endif
