#ifndef DRIVE_PERF_H_
#define DRIVE_PERF_H_

#include "drv_cmn.h"

#ifdef cplusplus
extern "C" {
#endif

void hw_perf_init(uint32_t corefreq);
uint32_t hw_perf_gettime(void);
uint32_t hw_perf_getfreq(void);

#ifdef cplusplus
}
#endif

#endif
