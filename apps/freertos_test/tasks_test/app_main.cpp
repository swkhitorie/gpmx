
#include "board_config.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include <drivers/drv_hrt.h>

void debug1(void *p)
{
    int cnt = 1;
    for (;;) {
        printf("debug1 : send \r\n");
        cnt++;
        // if (cnt == 5) {
        //     vTaskSuspend(NULL);
        // }
        vTaskDelay(100);
    }
}

static char pstr1[1024];
static char pstr2[1024];
void debug2(void *p)
{
    for (;;) {

        vTaskGetRunTimeStats(pstr1);
        vTaskList(pstr2);

        printf("\r\n%s \r\n", pstr1);
        printf("\r\n%s \r\n", pstr2);

        vTaskDelay(1000);
    }
}

void g_start(void *p)
{
    board_bsp_init();
    hrt_init();

    xTaskCreate(debug1, "debug1", 512, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 512, NULL, 3, NULL);

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
