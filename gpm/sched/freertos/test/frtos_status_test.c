#include "frtos_test.h"

#include "driver/mlog.h"

#include "FreeRTOS.h"

#include "task.h"
#include "fqueue.h"

#include <string.h>

int list_queue_dbg(void)
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
        KMRAW("[queue] %-18s,ptr:%p, curMsgs:%03u, maxUsedMsgs:%03u, capacity:%03u, item_size:%03u\r\n",
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

int list_rtos_dbg(void)
{
    TaskStatus_t *p_tasks;
    int sz, i;
    char status;

    uint64_t total_run = 0;
    uint64_t total_cnter = 0;
    uint64_t idle_cnter = 0;
    uint32_t free_heap_sz = xPortGetFreeHeapSize();
    static uint32_t list_rtos_cnter = 0;
    TaskTrace_t tasktrace;

    sz = uxTaskGetNumberOfTasks();
    p_tasks = (TaskStatus_t *)pvPortMalloc(sz * sizeof(TaskStatus_t));
    if (!p_tasks) {
        return -1;
    }

    // It will miss The deleted task runtimer 
    sz = uxTaskGetSystemState(p_tasks, sz, &total_run);

    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        if (strncmp(status->pcTaskName,"IDLE",4)) {
            total_cnter += status->ulRunTimeCounter;
        }
    }

    KMRAW("[main]CPU usage: %3.2f - Clock freq %u\r\n",
        (float) total_cnter / (total_run+1) * 100.0f, configCPU_CLOCK_HZ);

    KMRAW("[main]MEM usage: heap total size %8d, req %8d, free %8d\r\n",
        configTOTAL_HEAP_SIZE, configTOTAL_HEAP_SIZE - free_heap_sz, free_heap_sz);

    KMRAW("[main]Timenow: %10u, delta: %4dmsec, RTOS tick %8d, RTOS delta %4d\r\n",
        xTaskGetTickCount()*portTICK_PERIOD_MS, 
        (xTaskGetTickCount() - list_rtos_cnter)*portTICK_PERIOD_MS, 
        xTaskGetTickCount(),
        (xTaskGetTickCount() - list_rtos_cnter));

    KMRAW("[main]CS: %8d, IRQs num: %d, IRQs time: %d\r\n", 0, 0, 0);

    char task_state[] = {'X','R','B','S','D'};

    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        TaskHandle_t ptcb = status->xHandle;

        uxTaskStatus(ptcb, &tasktrace);

        float cpu_usage = (float)(status->ulRunTimeCounter) / (float)total_run * 100.0f;
        float stack_usage = (tasktrace.stackSize > 0) ? ((float)tasktrace.stackUsed / (float)tasktrace.stackSize * 100.0f) : 0.0f;

        KMRAW("[task] %15s %c, pri:%2lu, bptr=%x, wptr=%x, size=%6lu, used=%6lu, %5.1f, cpu:%.2f\r\n",
            status->pcTaskName,
            task_state[status->eCurrentState],
            (unsigned long) (status->uxCurrentPriority),
            tasktrace.stackBase,        // bptr
            tasktrace.stackHighWater,   // wptr
            (unsigned long) tasktrace.stackSize,
            (unsigned long) tasktrace.stackUsed,
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

        list_rtos_dbg();
        list_queue_dbg();

        vTaskDelay(1000);
    }
}

int freertos_status_test(int argc, char **argv)
{
    extern int freertos_task_test(int argc, char **argv);
    extern int freertos_queue_test(int argc, char **argv);

    freertos_task_test(0, NULL);
    freertos_queue_test(0, NULL);

    xTaskCreate(status_detect, "status", 1024, NULL, 3, NULL);

    return 0;
}
