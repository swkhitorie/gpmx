
#ifndef MMU_FREERTOS_HEAP4_H_
#define MMU_FREERTOS_HEAP4_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void   fr4_mem_init(void *mem, size_t size);
void  *fr4_mem_malloc(size_t sz);
void   fr4_mem_free(void *pv);
size_t fr4_mem_get_freeheapsize(void);
size_t fr4_mem_get_minimumEverfreeheapSize(void);

#ifdef __cplusplus
}
#endif

#endif
