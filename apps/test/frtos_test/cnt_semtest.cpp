#include "board_config.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t c_sem = NULL;

void debug1(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        if (cnt == 9) {
            printf("release sem twice \n");
            int ret = xSemaphoreGive(c_sem);
            ret = xSemaphoreGive(c_sem);
        }
        vTaskDelay(1000);
    }
}

void debug2(void *p)
{
    for (;;) {
        int ret = xSemaphoreTake(c_sem, 0);
        printf("debug2: task c_sem nonblock ret: %d \n", ret);
        vTaskDelay(400);
    }
}

int main(void)
{
    board_init();

    taskENTER_CRITICAL(); 
    c_sem = xSemaphoreCreateCounting(5, 5); // max cnt, init cnt
    xTaskCreate(debug1, "debug1", 256, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
