#include "board_config.h"

void NMI_Handler(void) {}
void HardFault_Handler(void) {}
void MemManage_Handler(void) {}
void BusFault_Handler(void) {}
void UsageFault_Handler(void) {}
void DebugMon_Handler(void) {}

// void PPP_IRQHandler(void)
// {
// }

#include "FreeRTOS.h"
#include "task.h"
void SysTick_Handler(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
    HAL_IncTick();
}

// void SVC_Handler(void)
// {
// }

// void PendSV_Handler(void)
// {
// }