#include <board_config.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"


#define EVENT_BIT_0  (1 << 0)
#define EVENT_BIT_1  (1 << 1)

EventGroupHandle_t xEventGroup = NULL;

void vTask1(void *p)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod0 = pdMS_TO_TICKS(2000);
    const TickType_t xPeriod1 = pdMS_TO_TICKS(3000);

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod0);
        xEventGroupSetBits(xEventGroup, EVENT_BIT_0);
        TEST_PRINTF("Task1: Set EVENT_BIT_0\n");

        vTaskDelayUntil(&xLastWakeTime, xPeriod1);
        xEventGroupSetBits(xEventGroup, EVENT_BIT_1);
        TEST_PRINTF("Task1: Set EVENT_BIT_1\n");
    }
}

void vTask2(void *p)
{
    EventBits_t uxBits;
    const EventBits_t uxBitsToWaitFor = EVENT_BIT_0 | EVENT_BIT_1;

    for (;;) {
        uxBits = xEventGroupWaitBits(xEventGroup,
                                    uxBitsToWaitFor,
                                    pdTRUE,
                                    pdTRUE,
                                    portMAX_DELAY);

        TEST_PRINTF("Task2: Both bits set! Received bits: 0x%x\n", uxBits);
    }
}

int freertos_eventgroup_test(int argc, char **argv)
{
    xEventGroup = xEventGroupCreate();
    configASSERT(xEventGroup != NULL);

    xTaskCreate(vTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task2", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    return 0;
}
