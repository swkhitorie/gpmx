#include "board_config.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t b_sem = NULL;

void debug1(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        if (cnt == 9) {
            printf("release sem twice \n");
            int ret = xSemaphoreGive(b_sem); //release
            ret = xSemaphoreGive(b_sem); //release
        }
        vTaskDelay(1000);
    }
}

void debug2(void *p)
{
    for (;;) {
        printf("debug2: task b_sem blocking \n");
        int ret = xSemaphoreTake(b_sem, portMAX_DELAY); // get, wait block
        vTaskDelay(10);
    }
}

int main(void)
{
    board_init();

    taskENTER_CRITICAL(); 
    b_sem = xSemaphoreCreateBinary();
    xTaskCreate(debug1, "debug1", 256, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
