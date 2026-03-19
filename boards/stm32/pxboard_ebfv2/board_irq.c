#include "board_config.h"

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#endif

#if defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>
#endif

void NMI_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void MemManage_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void BusFault_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void UsageFault_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }
void DebugMon_Handler(void) { BOARD_CRUSH_PRINTF("%s\r\n", __func__); }

#if !defined(CONFIG_MODULE_CMBACKTRACE)
void HardFault_Handler(void) 
{
    while (1) {
        BOARD_CRUSH_PRINTF("%s\r\n", __func__); 
        for(int i = 10000;i>0;i--)
        for(int j = 1000;j>0;j--);
    }
}
#endif

#if !defined(CONFIG_FREERTOS_ENABLE) && !defined(CONFIG_RTTNANO_ENABLE)
void PendSV_Handler(void) {}
#endif

#if !defined(CONFIG_FREERTOS_ENABLE)
void SVC_Handler(void) {}
#endif

void SysTick_Handler(void)
{
#if defined(CONFIG_FREERTOS_ENABLE)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        extern void xPortSysTickHandler(void);
        xPortSysTickHandler();
    }
#endif

#if defined(CONFIG_RTTNANO_ENABLE)
    rt_interrupt_enter();
    rt_tick_increase();
#endif

    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
        HAL_IncTick();
    }

#if defined(CONFIG_RTTNANO_ENABLE)
    rt_interrupt_leave();
#endif
}
