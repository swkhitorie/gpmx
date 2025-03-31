
#ifndef MMU_RTX_H_
#define MMU_RTX_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t rtx_mem_init(void *mem, uint32_t size);

void *rtx_mem_alloc(void *mem, uint32_t size, uint32_t type);

uint32_t rtx_mem_free(void *mem, void *block);

#ifdef __cplusplus
}
#endif

#endif
