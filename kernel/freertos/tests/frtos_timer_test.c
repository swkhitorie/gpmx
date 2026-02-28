
#include "freertos_test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

static TimerHandle_t stimer_1 = NULL;
static TimerHandle_t stimer_2 = NULL;
static uint32_t t_cnt1 = 0;
static uint32_t t_cnt2 = 0;

void stimer1_callback(void *arg)
{
    t_cnt1++;
    TickType_t tick1;
    tick1 = xTaskGetTickCount();
    TEST_PRINTF("stime1 call: %d \n", tick1);
}

void stimer2_callback(void *arg)
{
    t_cnt2++;
    TickType_t tick2;
    tick2 = xTaskGetTickCount();
    TEST_PRINTF("stime2 call: %d \n", tick2);
}

int freertos_timer_test(int argc, char **argv)
{
    stimer_1 = xTimerCreate("AutoReloadTimer", 1000, pdTRUE, 
        (void *)1, (TimerCallbackFunction_t)stimer1_callback); 
    xTimerStart(stimer_1, 0);  // start now

    stimer_2 = xTimerCreate("OneShotTimer", 5000, pdFALSE, 
        (void *)2, (TimerCallbackFunction_t)stimer2_callback); 
    xTimerStart(stimer_2, 0);  // start now

    return 0;
}
