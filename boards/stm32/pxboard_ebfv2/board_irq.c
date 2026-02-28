#include "board_config.h"

void NMI_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void MemManage_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void BusFault_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void UsageFault_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void DebugMon_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }

#ifndef CONFIG_MODULE_CMBACKTRACE
void HardFault_Handler(void) 
{
    while (1) {
        BOARD_CRUSH_PRINTF("%s\r\n", __func__); 
        for(int i = 10000;i>0;i--)
        for(int j = 1000;j>0;j--);
    }
}
#endif

#ifndef CONFIG_FREERTOS_ENABLE
void SVC_Handler(void) {}
void PendSV_Handler(void) {}
#else
#include <FreeRTOS.h>
#include <task.h>
extern void xPortSysTickHandler(void);
#endif

void SysTick_Handler(void)
{
#ifdef CONFIG_FREERTOS_ENABLE
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
#endif
    __disable_irq();
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
        HAL_IncTick();
    }
    __enable_irq();
}
