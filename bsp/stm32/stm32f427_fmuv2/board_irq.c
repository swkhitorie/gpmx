#include "board_config.h"

void NMI_Handler(void) {}
void HardFault_Handler(void) {}
void MemManage_Handler(void) {}
void BusFault_Handler(void) {}
void UsageFault_Handler(void) {}
void DebugMon_Handler(void) {}

#ifndef BSP_MODULE_FR
void SVC_Handler(void) {}
void PendSV_Handler(void) {}
#endif

#ifdef BSP_MODULE_FR
#include <FreeRTOS.h>
#include <task.h>
#endif
void SysTick_Handler(void)
{
#ifdef BSP_MODULE_FR
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
#endif
    HAL_IncTick();
}
