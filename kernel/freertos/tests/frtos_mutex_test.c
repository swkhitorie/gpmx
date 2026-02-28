#include "freertos_test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t mutex = NULL;

void mutex_t1(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        TEST_PRINTF("mutex_t1: low take mutex blocking \n");
        int ret = xSemaphoreTake(mutex, portMAX_DELAY);
        TEST_PRINTF("mutex_t1: low mutex taked %d \n", ret);

        for(int i=0;i<2000000;i++) {
            //  low priority hold mutex
			taskYIELD();
		}

        TEST_PRINTF("mutex_t1: low give mutex \n");
        ret = xSemaphoreGive(mutex);
        vTaskDelay(1000);
    }
}

void mutex_mid(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;
        TEST_PRINTF("mutex_mid task take sem blocking \n");

        vTaskDelay(100);
    }
}

void mutex_t2(void *p)
{
    int cnt = 1;
    for (;;) {
        TEST_PRINTF("mutex_t2: high take mutex blocking \n");
        int ret = xSemaphoreTake(mutex, portMAX_DELAY);
        TEST_PRINTF("mutex_t2: high mutex taked %d \n", ret);
    
        TEST_PRINTF("mutex_t2: high give mutex \n");
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
 * 
 * 
 * But this Mutex use example is not recommanded
 *     Mutex is Only for creating Critical section
 */
int freertos_mutex_test(int argc, char **argv)
{
    mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(mutex);
    xTaskCreate(mutex_t1, "mutex_t1", 256, NULL, 1, NULL);
    xTaskCreate(mutex_mid, "mutex_mid", 256, NULL, 2, NULL);
    xTaskCreate(mutex_t2, "mutex_t2", 256, NULL, 4, NULL);

    return 0;
}
