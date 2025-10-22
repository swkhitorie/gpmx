#include "n32g430.h"

void NMI_Handler(void) { while(1) {} }
void MemManage_Handler(void) { while(1) {} }
void BusFault_Handler(void) { while(1) {} }
void UsageFault_Handler(void) { while(1) {} }
void DebugMon_Handler(void) { while(1) {} }

void SVC_Handler(void) {}
void PendSV_Handler(void) {}

void HardFault_Handler(void)
{
    while (1) {
        // printf("hardfault \r\n");
    }
}


