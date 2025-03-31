#ifndef __MMU_TRACER_H_
#define __MMU_TRACER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void mmu_tracerecord(void *addr, uint32_t len);

void mmu_traceremove(void *addr);

void mmu_traceinfo(uint16_t idx, void **addr, uint32_t *len);

#ifdef __cplusplus
}
#endif

#endif
