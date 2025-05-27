#include "board_config.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t b_sem = NULL;

void low_task(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        printf("low task take sem blocking \n");
		int ret = xSemaphoreTake(b_sem, portMAX_DELAY);
        printf("low task sem taked %d \n", ret);
    
        for(int i=0;i<2000000;i++){
			taskYIELD();
		}
    
        printf("low task give sem \n");
        ret = xSemaphoreGive(b_sem);
        vTaskDelay(1000);
    }
}

void mid_task(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;
        printf("mid task take sem blocking \n");

        vTaskDelay(500);
    }
}

void high_task(void *p)
{
    for (;;) {
        printf("high task take sem blocking \n");
		int ret = xSemaphoreTake(b_sem, portMAX_DELAY);
        printf("high task sem taked %d \n", ret);
    
        printf("high task give sem \n");
        ret = xSemaphoreGive(b_sem);
        vTaskDelay(200);
    }
}

/**
 * L get sem
 * H try get sem, but sem holded by L
 * M can preempt L, make L can not give sem, H wait for long time
 */
int main(void)
{
    board_init();
    board_bsp_init();

    taskENTER_CRITICAL(); 
    b_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(b_sem);

    xTaskCreate(low_task, "low", 256, NULL, 1, NULL);
    xTaskCreate(mid_task, "mid", 256, NULL, 2, NULL);
    xTaskCreate(high_task, "high", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
