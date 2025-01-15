#include "board_config.h"

void NMI_Handler(void) {}
void HardFault_Handler(void) {}
void MemManage_Handler(void) {}
void BusFault_Handler(void) {}
void UsageFault_Handler(void) {}
void DebugMon_Handler(void) {}
void SVC_Handler(void) {}
void PendSV_Handler(void) {}

// void PPP_IRQHandler(void)
// {
// }
#ifdef BSP_MODULE_FR
#include <FreeRTOS.h>
#include <tasks.h>
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
