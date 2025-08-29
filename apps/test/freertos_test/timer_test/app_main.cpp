#include "board_config.h"
#include <stdio.h>
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
    printf("stime1 call: %d \n", tick1);
}

void stimer2_callback(void *arg)
{
    t_cnt2++;
    TickType_t tick2;
    tick2 = xTaskGetTickCount();
    printf("stime2 call: %d \n", tick2);
}

void g_start(void *p)
{
    board_bsp_init();
    hrt_init();

    stimer_1 = xTimerCreate("AutoReloadTimer", 1000, pdTRUE, 
        (void *)1, (TimerCallbackFunction_t)stimer1_callback); 
    xTimerStart(stimer_1, 0);  // start now

    stimer_2 = xTimerCreate("OneShotTimer", 5000, pdFALSE, 
        (void *)2, (TimerCallbackFunction_t)stimer2_callback); 
    xTimerStart(stimer_2, 0);  // start now

    vTaskDelete(NULL);
}

int main(void)
{
    board_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(g_start, "g_start", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}

