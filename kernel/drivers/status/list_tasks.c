#include "list_tasks.h"

#include "FreeRTOS.h"
#include "tasks.h"

int list_tasks(int argc, char **argv)
{
    TaskStatus_t *p_tasks;
    int sz, i;
    char status;

    sz = uxTaskGetNumberOfTasks();
    p_tasks = pvPortMalloc(sz * sizeof(TaskStatus_t));
    if (!p_tasks) {
        return -1;
    }

    sz = uxTaskGetSystemState(p_tasks, sz, NULL);

    uint32_t total_run = 0;
    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        total_run += status->ulRunTimeCounter;
    }

    if (total_run == 0) {
        total_run = 1;
    }

    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        TaskHandle_t *ptcb = &status->xHandle;

        uint32_t stack_sz = (uint32_t)ptcb->pxEndOfStack - (uint32_t)ptcb->pxStack;
        UBaseType_t high_mark = uxTaskGetStackHighWaterMark(ptcb);
        uint32_t stack_used = stack_sz - (high_mark * sizeof(StackType_t));

        float cpu_usage = (float)status->ulRunTimeCounter / (float)total_run * 100.0f;
        float stack_usage = (stack_sz > 0) ? ((float)stack_used / (float)stack_sz * 100.0f) : 0.0f;
        void *stack_base = (void *)ptcb->pxStack;    // bptr
        void *stack_hw = (void *)((uint32_t)stack_base + stack_used);

        // int len = snprintf(cPrintBuffer, sizeof(cPrintBuffer),
        //     "[main][task] name %15s %x - prio :%2lu, "
        //     "bptr=%p, wptr=%p, "
        //     "size=%6lu, used=%6lu -> %5.1f - CPU usage :%.2f",
        //     status->pcTaskName,
        //     status->eCurrentState,
        //     (unsigned long) status->uxCurrentPriority,
        //     stack_base,        // bptr
        //     stack_hw,   // wptr
        //     (unsigned long) stack_sz,
        //     (unsigned long) stack_used,
        //     (double) stack_usage,
        //     (double) cpu_usage
        // );
    }

    vPortFree(p_tasks);
    return 0;
}
