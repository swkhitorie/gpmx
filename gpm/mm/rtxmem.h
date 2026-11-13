
#ifndef __RTX_MEM_H_
#define __RTX_MEM_H_

#include <stdint.h>
#include <stddef.h>

#if !defined(UINTPTR_MAX) || \
    ((UINTPTR_MAX != 0xFFFFFFFFU) && (UINTPTR_MAX != 0xFFFFFFFFFFFFFFFFULL))
#error rtx memory manage module only supports 32-bit or 64-bit machines
#endif

//  memory block info: length = <31:2>:'00', type = <1:0>
#define MB_INFO_TYPE_MASK   ((size_t)0x3U)
#define MB_INFO_LEN_MASK    (~(size_t)MB_INFO_TYPE_MASK)

/* memory pool header */
typedef struct {
    size_t size;    /*<! memory pool size */
    size_t used;    /*<! used memory */
} mem_head_t;

/* memory block header */
typedef struct mem_block_s {
    struct mem_block_s *next;     /*<! next memory Block in list */
    size_t              info;     /*<! block info or max used memory (in last block) */
} mem_block_t;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t rtx_mem_init(void *mem, size_t size);
void    *rtx_mem_alloc(void *mem, size_t size, uint32_t type);
uint32_t rtx_mem_free(void *mem, void *block);
size_t   rtx_mem_used(void *mem);

#ifdef __cplusplus
}
#endif

#endif  // rtx_mem_H
