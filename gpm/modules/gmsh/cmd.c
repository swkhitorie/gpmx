#include "gmsh.h"
#include <string.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#include <fqueue.h>

int list_rtos(int argc, char **argv)
{
    TaskStatus_t *p_tasks;
    int sz, i;
    char status;

    uint32_t stack_base;
    uint32_t stack_hw;
    uint32_t stack_sz;
    uint32_t stack_used;

    uint64_t total_run = 0;
    uint64_t total_cnter = 0;
    uint32_t free_heap_sz = xPortGetFreeHeapSize();
    static uint32_t list_rtos_cnter = 0;

    sz = uxTaskGetNumberOfTasks();
    p_tasks = (TaskStatus_t *)pvPortMalloc(sz * sizeof(TaskStatus_t));
    if (!p_tasks) {
        return -1;
    }

    sz = uxTaskGetSystemState(p_tasks, sz, &total_run);
    if (total_run == 0) {
        total_run = 1;
    }

    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        if (strncmp(status->pcTaskName,"IDLE",4)) {
            total_cnter += status->ulRunTimeCounter;
        }
    }

    gsh_kprintf("[main]CPU usage: %3.2f - Clock freq %u\r\n",
        (float) total_cnter / total_run * 100.0f, configCPU_CLOCK_HZ);

    gsh_kprintf("[main]MEM usage: heap total size %8d, req %8d, free %8d\r\n",
        configTOTAL_HEAP_SIZE, configTOTAL_HEAP_SIZE - free_heap_sz, free_heap_sz);

    gsh_kprintf("[main]Timenow: %10u, delta: %4dmsec, RTOS tick %8d, RTOS delta %4d\r\n",
        xTaskGetTickCount()*portTICK_PERIOD_MS, 
        (xTaskGetTickCount() - list_rtos_cnter)*portTICK_PERIOD_MS, 
        xTaskGetTickCount(),
        (xTaskGetTickCount() - list_rtos_cnter));

    gsh_kprintf("[main]CS: %8d, IRQs num: %d, IRQs time: %d\r\n", 0, 0, 0);

    char task_state[] = {'X','R','B','S','D'};

    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        TaskHandle_t ptcb = status->xHandle;

        uxTaskStatus(ptcb, &stack_base, &stack_hw, &stack_sz, &stack_used);

        float cpu_usage = (float)(status->ulRunTimeCounter) / (float)total_run * 100.0f;
        float stack_usage = (stack_sz > 0) ? ((float)stack_used / (float)stack_sz * 100.0f) : 0.0f;

        gsh_kprintf("[main][task] name %15s %c - prio :%2lu, bptr=%x, wptr=%x, size=%6lu, used=%6lu -> %5.1f - CPU usage :%.2f\r\n",
            status->pcTaskName,
            task_state[status->eCurrentState],
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
MSH_FUNCTION_EXPORT_CMD(list_rtos, list_tasks, get rtos tasks status);

int list_queue(int argc, char **argv)
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
        gsh_kprintf("[main][queue] %-18s,ptr:%p, curMsgs:%03u, maxUsedMsgs:%03u, capacity:%03u, item_size:%03u\r\n",
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
MSH_FUNCTION_EXPORT_CMD(list_queue, list_queues, get rtos queue status);

int free_handle(int argc, char **argv)
{
    size_t total = configTOTAL_HEAP_SIZE;
    size_t free_bytes = xPortGetFreeHeapSize();
    size_t used = total - free_bytes;
    size_t largest = 0, nfree = 0, nused = 0;

    vPortGetHeapStats(&largest, &nfree, &nused);

    gsh_kprintf("                   total       used       free    largest  nused  nfree\n");
    gsh_kprintf("        Umem: %10lu %10lu %10lu %10lu %5lu %5lu\n",
            total, used, free_bytes, largest, nused, nfree);
}
MSH_FUNCTION_EXPORT_CMD(free_handle, free, get rtos memory manage status);
#endif

#if defined(CONFIG_MODULE_KPOSIX_TIME)
#include <time.h>
#include <stdlib.h>
#include "gpm/rtc.h"

int systime_handle(int argc, char **argv)
{
    if (argc > 2 && !strcmp(argv[1], "set")) {
        struct timespec real_time;
        real_time.tv_sec = (time_t)strtoul(argv[2], NULL, 10);
        clock_settime(CLOCK_REALTIME, &real_time);

        gsh_kprintf("[system_time] epoch time set completed: %u\r\n", real_time.tv_sec);
    }

    if (argc == 2 && !strcmp(argv[1], "get")) {
        struct timespec real_time;
        struct timespec boot_time;
        time_t now_time;
        clock_gettime(CLOCK_REALTIME, &real_time);
        clock_gettime(CLOCK_MONOTONIC, &boot_time);

        now_time = real_time.tv_sec;
        struct rtc_tm now_tm;
        rtc_timstamp_to_tm((rtc_time_t)now_time, &now_tm);

        gsh_kprintf("[system_time] Unix epoch time: %u\r\n", real_time.tv_sec);
        gsh_kprintf("[system_time] System time: Tue %04d-%02d-%02d %02d:%02d:%02d\r\n",
            now_tm._year+1900, now_tm._mon+1, now_tm._mday,
            now_tm._hour, now_tm._min, now_tm._sec);
        gsh_kprintf("[system_time] Uptime (since boot): %u s\r\n", boot_time.tv_sec);
    }
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(systime_handle, system_time, handle the systime);
#endif
