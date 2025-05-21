#include "board_config.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t t_queue = NULL;

void debug1(void *p)
{
    int cnt = 0;
    uint32_t send_data1 = 1;
    uint32_t send_data2 = 2;
    for (;;) {
        printf("debug1 : queue send \r\n");
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

void debug2(void *p)
{
    uint32_t r_queue;
    for (;;) {
        int ret = xQueueReceive(t_queue, &r_queue, portMAX_DELAY); // block waiting
        printf("recv: %d \n", r_queue);
        vTaskDelay(10);
    }
}

int main(void)
{
    board_init();

    taskENTER_CRITICAL(); 
    t_queue = xQueueCreate(3, 4); //3 items, 4 byte
    xTaskCreate(debug1, "debug1", 256, NULL, 3, NULL);
    xTaskCreate(debug2, "debug2", 256, NULL, 3, NULL);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();
    for (;;);
}
