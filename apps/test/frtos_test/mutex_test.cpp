#include "board_config.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t mutex = NULL;

void debug1(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        printf("debug1: low take mutex blocking \n");
        int ret = xSemaphoreTake(mutex, portMAX_DELAY);
        printf("debug1: low mutex taked %d \n", ret);

        for(int i=0;i<2000000;i++) {
            //  low priority hold mutex
			taskYIELD();
		}

        printf("debug1: low give mutex \n");
        ret = xSemaphoreGive(mutex);
        vTaskDelay(1000);
    }
}

void mid_task(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;
        printf("mid task take sem blocking \n");

        vTaskDelay(10);
    }
}

void debug2(void *p)
{
    int cnt = 1;
    for (;;) {
        printf("debug 2: high take mutex blocking \n");
        int ret = xSemaphoreTake(mutex, portMAX_DELAY);
        printf("debug 2: high mutex taked %d \n", ret);
    
        printf("debug 2: high give mutex \n");
        ret = xSemaphoreGive(mutex);
        vTaskDelay(1000);
    }
}

/**
 * Mutex has a priority inheritance, 
 * which is the biggest difference from cnt semaphore
 * 
 * when L take mutex success and H block wait mutex
 * the L priority add same with H,
 * then M can not preempt L
 */
int main(void)
{
    board_init();

    taskENTER_CRITICAL(); 
    mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(mutex);
    xTaskCreate(debug1, "debug1", 256, NULL, 1, NULL);
    xTaskCreate(mid_task, "mid", 256, NULL, 2, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 4, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
