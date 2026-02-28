#include "freertos_test.h"
#include "FreeRTOS.h"

#include "task.h"
#include "queue.h"

int list_queue(void)
{
    int size;
    QueueRegistryItem_t *plist;
    QueueRegistryItem_t *item;

    uint32_t current_msg_wait = 0; // = uxQueueMessagesWaiting(pxQueue); // pxQueue->uxMessagesWaiting;
    uint32_t max_msg_wait = 0; //pxQueue->uxMaxMessagesWaiting;
    uint32_t queue_capacity = 0; //pxQueue->uxLength;
    uint32_t queue_item_size = 0; //pxQueue->uxItemSize;

    size = uxQueueRegistyListGet(&plist);

    for (int i = 0; i < configQUEUE_REGISTRY_SIZE; i++) {

        item = &plist[i];

        if (item->xHandle == NULL) {
            continue;
        }

        uxQueueStatus(item->xHandle, &current_msg_wait, &max_msg_wait, &queue_capacity, &queue_item_size);
        TEST_PRINTF("[main][queue] %-18s,ptr:%p, curMsgs:%03u, maxUsedMsgs:%03u, capacity:%03u, item_size:%03u\r\n",
            pcQueueGetName(item->xHandle),
            (void*)item->xHandle,
            current_msg_wait,
            max_msg_wait,
            queue_capacity,
            queue_item_size
        );
    }

    return 0;
}

int list_rtos(void)
{
    TaskStatus_t *p_tasks;
    int sz, i;
    char status;

    uint32_t stack_base;
    uint32_t stack_hw;
    uint32_t stack_sz;
    uint32_t stack_used;

    uint32_t total_run = 0;
    uint32_t total_cnter = 0;
    uint32_t free_heap_sz = xPortGetFreeHeapSize();
    static uint32_t list_rtos_cnter = 0;

    sz = uxTaskGetNumberOfTasks();
    p_tasks = pvPortMalloc(sz * sizeof(TaskStatus_t));
    if (!p_tasks) {
        return -1;
    }

    sz = uxTaskGetSystemState(p_tasks, sz, &total_run);
    if (total_run == 0) {
        total_run = 1;
    }

    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        total_cnter += status->ulRunTimeCounter;
    }

    TEST_PRINTF("[main]CPU usage: %3.2f - Clock freq %u\r\n",
        (float) total_cnter / total_run, configCPU_CLOCK_HZ);

    TEST_PRINTF("[main]MEM usage: heap total size %8d, req %8d, free %8d\r\n",
        configTOTAL_HEAP_SIZE, configTOTAL_HEAP_SIZE - free_heap_sz, free_heap_sz);

    TEST_PRINTF("[main]Timenow: %10u, delta: %4dmsec, RTOS tick %8d, RTOS delta %4d\r\n",
        xTaskGetTickCount()*portTICK_PERIOD_MS, 
        (xTaskGetTickCount() - list_rtos_cnter)*portTICK_PERIOD_MS, 
        xTaskGetTickCount(),
        (xTaskGetTickCount() - list_rtos_cnter));

    //gpOS_kernel_get_context_switch_number()
    //gpOS_kernel_get_interrupts_occurred_number()
    //gpOS_kernel_get_interrupts_spent_time()
    TEST_PRINTF("[main]CS: %8d, IRQs num: %d, IRQs time: %d\r\n", 0, 0, 0);

/*
	eRunning = 0
	eReady = 1
	eBlocked = 2
	eSuspended = 3
	eDeleted = 4
*/
    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        TaskHandle_t ptcb = status->xHandle;

        uxTaskStatus(ptcb, &stack_base, &stack_hw, &stack_sz, &stack_used);

        float cpu_usage = (float)(status->ulRunTimeCounter) / (float)total_run * 100.0f;
        float stack_usage = (stack_sz > 0) ? ((float)stack_used / (float)stack_sz * 100.0f) : 0.0f;

        TEST_PRINTF("[main][task] name %15s %x - prio :%2lu, bptr=%x, wptr=%x, size=%6lu, used=%6lu -> %5.1f - CPU usage :%.2f\r\n",
            status->pcTaskName,
            status->eCurrentState,
            (unsigned long) (status->uxCurrentPriority),
            stack_base,        // bptr
            stack_hw,   // wptr
            (unsigned long) stack_sz,
            (unsigned long) stack_used,
            (double) stack_usage,
            (double) cpu_usage
        );
    }

    vPortFree(p_tasks);

    list_rtos_cnter = xTaskGetTickCount();
    return 0;
}

void status_detect(void *p)
{

    for (;;) {

        list_rtos();
        list_queue();

        vTaskDelay(1000);
    }
}

int freertos_status_test(int argc, char **argv)
{
    freertos_task_test(0, NULL);
    freertos_queue_test(0, NULL);

    xTaskCreate(status_detect, "status", 1024, NULL, 3, NULL);

    return 0;
}
