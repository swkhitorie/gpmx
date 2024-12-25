/**
 * low level driver for stm32f4 series, base on std periph library
 * module for core kernel
*/

#include "lld_kernel.h"

static RCC_ClocksTypeDef rcc_clocks;
static volatile uint32_t us_ticks = 0;
static volatile uint32_t systick_ticks = 0;

void lld_kernel_init(uint32_t init)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	SysTick_Config(SystemCoreClock / 1e3);
	RCC_GetClocksFreq(&rcc_clocks);
	us_ticks = rcc_clocks.SYSCLK_Frequency / 1e6;    
}

uint64_t lld_kernel_get_time(uint32_t way)
{    
    if (way == 0) {
        uint64_t m0 = systick_ticks;
        volatile uint64_t u0 = SysTick->VAL;
        const uint64_t tms = SysTick->LOAD + 1;
        volatile uint64_t abs_time = (m0 * 1000 + ((tms - u0) * 1000) / tms);
        return abs_time;
    }

    return 0;
}

void lld_kernel_irq()
{
    systick_ticks++;
}

