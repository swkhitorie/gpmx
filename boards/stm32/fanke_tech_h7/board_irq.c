#include "board_config.h"

#include <stdio.h>

void NMI_Handler(void) { printf("%s\r\n", __func__); }
void MemManage_Handler(void) { printf("%s\r\n", __func__); }
void BusFault_Handler(void) { printf("%s\r\n", __func__); }
void UsageFault_Handler(void) { printf("%s\r\n", __func__); }
void DebugMon_Handler(void) { printf("%s\r\n", __func__); }

static int readlr()
{
    uint32_t raddr;
#ifdef __GNUC__
    __asm volatile ("mov %0,lr":"=r"(raddr));
#endif
    return raddr;
}

void HardFault_Handler(void) 
{
    uint32_t lr = readlr();
    uint32_t hfsr = *(volatile uint32_t *)(0xE000ED2C);
    uint32_t cfsr = *(volatile uint32_t *)(0xE000ED28);
    uint32_t bfar = *(volatile uint32_t *)(0xE000ED38);
    uint32_t sp = 0;
    uint32_t stacked_r0;
    uint32_t stacked_r1;
    uint32_t stacked_r2;
    uint32_t stacked_r3;
    uint32_t stacked_r12;
    uint32_t stacked_lr;
    uint32_t stacked_pc;
    uint32_t stacked_psr;

    if (lr & 0x00000004) {
        sp = __get_PSP();
    } else {
        sp = __get_MSP();
    }

    stacked_r0 = *(uint32_t *)(sp+0);
    stacked_r1 = *(uint32_t *)(sp+4);
    stacked_r2 = *(uint32_t *)(sp+8);
    stacked_r3 = *(uint32_t *)(sp+12);
    stacked_r12 = *(uint32_t *)(sp+16);
    stacked_lr = *(uint32_t *)(sp+20);
    stacked_pc = *(uint32_t *)(sp+24);
    stacked_psr = *(uint32_t *)(sp+32);

    while (1) {

        printf("%s LR:0x%08X, PC:0x%08X HFSR:0x%08X CFSR:0x%08X BFAR:0x%08X\r\n", 
            __func__, 
            lr,
            stacked_pc, 
            hfsr, 
            cfsr, 
            bfar); 

        for(int i = 10000;i>0;i--)
        for(int j = 1000;j>0;j--);
    }

}

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
extern void xPortSysTickHandler(void);
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
