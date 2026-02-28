#include "freertos_test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t d_sem = NULL;

void prv_tlow(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        TEST_PRINTF("prv low task take sem blocking \n");
		int ret = xSemaphoreTake(d_sem, portMAX_DELAY);
        TEST_PRINTF("prv low task sem taked %d \n", ret);

        for(int i=0;i<2000000;i++){
			taskYIELD();
		}

        TEST_PRINTF("prv low task give sem \n");
        ret = xSemaphoreGive(d_sem);
        vTaskDelay(1000);
    }
}

void prv_tmid(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;
        TEST_PRINTF("prv mid task take sem blocking \n");

        vTaskDelay(500);
    }
}

void prv_thigh(void *p)
{
    for (;;) {
        TEST_PRINTF("prv high task take sem blocking \n");
		int ret = xSemaphoreTake(d_sem, portMAX_DELAY);
        TEST_PRINTF("prv high task sem taked %d \n", ret);
    
        TEST_PRINTF("prv high task give sem \n");
        ret = xSemaphoreGive(d_sem);
        vTaskDelay(200);
    }
}

/**
 * L get sem
 * H try get sem, but sem holded by L
 * M can preempt L, make L can not give sem, H wait for long time
 */
int freertos_priority_reverse_test(int argc, char **argv)
{
    d_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(d_sem);

    xTaskCreate(prv_tlow, "prv_low_task", 256, NULL, 1, NULL);
    xTaskCreate(prv_tmid, "prv_mid_task", 256, NULL, 2, NULL);
    xTaskCreate(prv_thigh, "prv_high_task", 256, NULL, 3, NULL);

    return 0;
}
