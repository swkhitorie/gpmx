#include "board_config.h"

void board_init()
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    if (SysTick_Config(SystemCoreClock / 1000U)) { while (1){} }
    NVIC_SetPriority(SysTick_IRQn, 0x01);
}

uint32_t board_tickget()
{
    return CoreTick;
}

