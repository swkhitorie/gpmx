#include "freertos_test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t b_sem = NULL;
SemaphoreHandle_t c_sem = NULL;

void sem_t1(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;
        if (cnt % 5 == 0) {
            TEST_PRINTF("\nsem_t1: release binary b_sem \n");
            int ret = xSemaphoreGive(b_sem); //release
        }
        vTaskDelay(1000);
    }
}

void sem_t2(void *p)
{
    for (;;) {
        TEST_PRINTF("\nsem_t2: task binary b_sem blocking \n");
        int ret = xSemaphoreTake(b_sem, portMAX_DELAY); // get, wait block
        vTaskDelay(10);
    }
}

void sem_t3(void *p)
{
    int cnt = 1;
    for (;;) {
        cnt++;

        if (cnt % 6 == 0) {
            TEST_PRINTF("\nsem_t3: release cnt c_sem twice \n");
            int ret = xSemaphoreGive(c_sem);
            ret = xSemaphoreGive(c_sem);
        }
        vTaskDelay(1000);
    }
}

void sem_t4(void *p)
{
    for (;;) {
        int ret = xSemaphoreTake(c_sem, 0);
        TEST_PRINTF("\nsem_t4: task cnt c_sem nonblock ret: %d \n", ret);
        vTaskDelay(400);
    }
}

/**
 * semphr cnt and binary test
 */
int freertos_sem_test(int argc, char **argv)
{
    b_sem = xSemaphoreCreateBinary();
    c_sem = xSemaphoreCreateCounting(5, 5); // max cnt, init cnt
    xTaskCreate(sem_t1, "sem_t1", 256, NULL, 3, NULL);
    xTaskCreate(sem_t2, "sem_t2", 256, NULL, 3, NULL);
    xTaskCreate(sem_t3, "sem_t3", 256, NULL, 3, NULL);
    xTaskCreate(sem_t4, "sem_t4", 256, NULL, 3, NULL);

    return 0;
}
