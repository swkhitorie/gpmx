
#ifndef __RTX_MEM_H_
#define __RTX_MEM_H_

#include <stdint.h>

#if (UINTPTR_MAX != 0xFFFFFFFFU)
#error rtx memory manage module only run 32-bits machine
#endif

//  memory block info: length = <31:2>:'00', type = <1:0>
#define MB_INFO_LEN_MASK        0xFFFFFFFCU
#define MB_INFO_TYPE_MASK       0x00000003U

// memory pool header
typedef struct {
    uint32_t size;    /*<! memory pool size */
    uint32_t used;    /*<! used memory */
} mem_head_t;

// memory block header
typedef struct mem_block_s {
    struct mem_block_s *next;     /*<! next memory Block in list */
    uint32_t            info;     /*<! block info or max used memory (in last block) */
} mem_block_t;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t rtx_mem_init(void *mem, uint32_t size);
void *rtx_mem_alloc(void *mem, uint32_t size, uint32_t type);
uint32_t rtx_mem_free(void *mem, void *block);
uint32_t rtx_mem_used(void *mem);

#ifdef __cplusplus
}
#endif

#endif  // rtx_mem_H
