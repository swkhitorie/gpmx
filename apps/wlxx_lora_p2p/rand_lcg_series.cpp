#include "rand_lcg_series.h"

static uint32_t rand_seed = 0;

void rand_lcg_seed_set(uint32_t val)
{
    rand_seed = val;
}

uint32_t rand_lcg_seed_get()
{
    return rand_seed;
}

uint32_t rand_lcg_seed_next(uint32_t list_len)
{
    uint32_t a = RAND_LCG_PARAM_A;
    uint32_t c = RAND_LCG_PARAM_C;

    rand_seed = (a * rand_seed + c);

    return rand_seed % list_len;
}
