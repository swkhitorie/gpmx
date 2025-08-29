#include "board_config.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t b_sem = NULL;
SemaphoreHandle_t c_sem = NULL;

void debug1(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;
        if (cnt % 5 == 0) {
            printf("\ndebug1: release binary b_sem \n");
            int ret = xSemaphoreGive(b_sem); //release
        }
        vTaskDelay(1000);
    }
}

void debug2(void *p)
{
    for (;;) {
        printf("\ndebug2: task binary b_sem blocking \n");
        int ret = xSemaphoreTake(b_sem, portMAX_DELAY); // get, wait block
        vTaskDelay(10);
    }
}

void debug3(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        if (cnt % 6 == 0) {
            printf("\ndebug3: release cnt c_sem twice \n");
            int ret = xSemaphoreGive(c_sem);
            ret = xSemaphoreGive(c_sem);
        }
        vTaskDelay(1000);
    }
}

void debug4(void *p)
{
    for (;;) {
        int ret = xSemaphoreTake(c_sem, 0);
        printf("\ndebug4: task cnt c_sem nonblock ret: %d \n", ret);
        vTaskDelay(400);
    }
}

void g_start(void *p)
{
    board_bsp_init();

    b_sem = xSemaphoreCreateBinary();
    c_sem = xSemaphoreCreateCounting(5, 5); // max cnt, init cnt
    xTaskCreate(debug1, "debug1", 256, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 3, NULL);
    xTaskCreate(debug3, "debug3", 256, NULL, 3, NULL);
    xTaskCreate(debug4, "debug4", 256, NULL, 3, NULL);
    vTaskDelete(NULL);
}

/**
 * semphr cnt and binary test
 */
int main(void)
{
    board_init();

    taskENTER_CRITICAL(); 
    xTaskCreate(g_start, "g_start", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
