#include "FreeRTOS.h"
#include "task.h"

StackType_t xTaskIdle_stack[configMINIMAL_STACK_SIZE];
StaticTask_t xTaskIdle;

StackType_t xTasktimer_stack[configTIMER_TASK_STACK_DEPTH];
StaticTask_t xTasktimer;

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
    StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xTaskIdle;
    *ppxIdleTaskStackBuffer = xTaskIdle_stack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, 
    StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTasktimer;
    *ppxTimerTaskStackBuffer = xTasktimer_stack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationMallocFailedHook( void )
{
    printf("[ERROR] memory allocate failed, free: %d bytes\r\n", xPortGetFreeHeapSize());
}
