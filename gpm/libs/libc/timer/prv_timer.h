#ifndef PRV_TIMER_H_
#define PRV_TIMER_H_

#include <gpmx/config.h>
#include <signal.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

typedef struct timer_internal {
    StaticTimer_t buff;
    struct sigevent event;
    TickType_t period;
} timer_internal_t;

#ifdef __cplusplus
extern "C" {
#endif

void prv_timer_callback(TimerHandle_t handle);

#ifdef __cplusplus
}
#endif

#endif

#endif
