#ifndef RAND_LCG_SERIES_H_
#define RAND_LCG_SERIES_H_

#include <stdint.h>

#define RAND_LCG_PARAM_A        (1664525)
#define RAND_LCG_PARAM_C        (1013904223)

#ifdef __cplusplus
extern "C" {
#endif

void rand_lcg_seed_set(uint32_t val);

uint32_t rand_lcg_seed_get();

uint32_t rand_lcg_seed_next(uint32_t list_len);

#ifdef __cplusplus
}
#endif

#endif
