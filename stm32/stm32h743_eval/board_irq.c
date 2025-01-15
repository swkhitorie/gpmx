#include "board_config.h"
#ifdef RTOS_FR
#include "FreeRTOS.h"
#include "task.h"
#endif

void NMI_Handler(void) {}
void HardFault_Handler(void) {}
void MemManage_Handler(void) {}
void BusFault_Handler(void) {}
void UsageFault_Handler(void) {}
void DebugMon_Handler(void) {}

// void PPP_IRQHandler(void)
// {
// }

void SysTick_Handler(void)
{
#ifdef RTOS_FR
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
#endif
    HAL_IncTick();
}

// void SVC_Handler(void)
// {
// }

// void PendSV_Handler(void)
// {
// }