#include "drv_perf.h"

static uint32_t g_core_freq = 0;

void hw_perf_init(uint32_t corefreq)
{
    g_core_freq = corefreq;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // unlock dwt module for cortex-m7
#if defined(DRV_STM32_H7)
    DWT->LAR = 0xC5ACCE55;
#endif

    DWT->CYCCNT = 0;

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t hw_perf_gettime(void)
{
    return DWT->CYCCNT;
}

uint32_t hw_perf_getfreq(void)
{
    return g_core_freq;
}

