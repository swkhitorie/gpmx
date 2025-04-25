#include "board_config.h"

void NMI_Handler(void) {}
void HardFault_Handler(void) { while(1) { BOARD_BLUE_LED(true);} }
void MemManage_Handler(void) {}
void BusFault_Handler(void) { while(1) { BOARD_RED_LED(true);} }
void UsageFault_Handler(void) {}
void DebugMon_Handler(void) {}
// void PPP_IRQHandler(void)
// {
// }

#ifndef CONFIG_BOARD_FREERTOS_ENABLE
void SVC_Handler(void) {}
void PendSV_Handler(void) {}
#endif

#ifdef CONFIG_BOARD_FREERTOS_ENABLE
#include <FreeRTOS.h>
#include <task.h>
#endif
void SysTick_Handler(void)
{
#ifdef CONFIG_BOARD_FREERTOS_ENABLE
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
#endif
    HAL_IncTick();
}
