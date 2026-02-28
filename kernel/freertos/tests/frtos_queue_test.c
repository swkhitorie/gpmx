#include "freertos_test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t t_queue = NULL;

void queue_t1(void *p)
{
    int cnt = 0;
    uint32_t send_data1 = 1;
    uint32_t send_data2 = 2;
    for (;;) {
        TEST_PRINTF("queue_t1 : queue send \r\n");
        cnt++;
        if (cnt == 5) {
            vTaskSuspend(NULL);
            // vTaskResume(NULL);
        }

        if (cnt == 2) {
            int ret = xQueueSend(t_queue, &send_data1, 0);
        } else if (cnt == 3) {
            int ret2 = xQueueSend(t_queue, &send_data2, 0);
        }

        vTaskDelay(1000);
    }
}

void queue_t2(void *p)
{
    uint32_t r_queue;
    for (;;) {
        int ret = xQueueReceive(t_queue, &r_queue, portMAX_DELAY); // block waiting
        TEST_PRINTF("queue_t2: %d \n", r_queue);
        vTaskDelay(10);
    }
}

int freertos_queue_test(int argc, char **argv)
{
    t_queue = xQueueCreate(3, 4); //3 items, 4 byte
    vQueueAddToRegistry(t_queue, "queue_test_1");
    xTaskCreate(queue_t1, "queue_t1", 256, NULL, 3, NULL);
    xTaskCreate(queue_t2, "queue_t2", 256, NULL, 3, NULL);

    return 0;
}
