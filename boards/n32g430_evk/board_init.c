#include <board_config.h>

static volatile uint32_t g_tick_cnt = 0;

void board_reboot()
{
    NVIC_SystemReset();
}

void SysTick_Handler(void)
{
    g_tick_cnt++;
}

uint32_t board_get_tmstamp(void)
{
    return g_tick_cnt;
}

void board_init()
{
    RCC_ClocksType rcc_clks;

    RCC_Clocks_Frequencies_Value_Get(&rcc_clks);
    SysTick_Config(rcc_clks.SysclkFreq / 1000);

    NVIC_Priority_Group_Set(NVIC_PER4_SUB0_PRIORITYGROUP);
}

void board_ms_delay(uint32_t ms)
{
    uint32_t tickstart = board_get_tmstamp();
    uint32_t wait = ms;

    while ((board_get_tmstamp() - tickstart) < wait) {

    }
}

uint32_t board_elapsed_tick(const uint32_t tick)
{
    uint32_t now = board_get_tmstamp();
    if (tick > now) {
        return 0;
    }
    return now - tick;
}
