#include <board_config.h>
#include <stdio.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#include <drivers/drv_hrt.h>

using namespace time_literals;

/**
 * drv_hrt support: f4, h7
 * this is debug example, do not call printf in hrt call, because it is in irq mode
 */
int call1_arg = 3;
struct hrt_call call1;
void hrt_call1(void *arg)
{
    float now = hrt_absolute_time()/1e6f;
    printf("%.6f call 1 run | iscalled?:%d | arg: %d | in irq?:%d \r\n", now,
         hrt_called(&call1), *(int *)arg, (__get_IPSR() != 0U));
}

float call2_arg = 2.78f;
struct hrt_call call2;
void hrt_call2(void *arg)
{
    float now = hrt_absolute_time()/1e6f;
    printf("%.6f call 2 run | iscalled?:%d | arg: %.5f \r\n", now, hrt_called(&call2), *(float *)arg);
}

double call3_arg = 3.141592678;
struct hrt_call call3;
void hrt_call3(void *arg)
{
    float now = hrt_absolute_time()/1e6f;
    printf("%.6f call 3 run | iscalled?:%d | arg: %.5f \r\n", now, hrt_called(&call3), *(double *)arg);
}

void debug1(void *p)
{
    for (;;) {
        // printf("debug1 : send \r\n");
        vTaskDelay(100);
    }
}

void g_start(void *p)
{
    board_bsp_init();
    hrt_init();

    //call1: delay 2s, period 1s
    //call2: in abstime 6.2s
    //call3: after 1s
    hrt_call_every(&call1, 2_s, 1_s, hrt_call1, &call1_arg);
    hrt_call_at(&call2, 6_s+200_ms, hrt_call2, &call2_arg);
    hrt_call_after(&call3, 1_s, hrt_call3, &call3_arg);

    xTaskCreate(debug1, "debug1", 512, NULL, 3, NULL);

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
