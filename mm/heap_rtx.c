
#include "heap_rtx.h"

#ifndef NULL
#define NULL (0)
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

mem_head_t *mem_head_ptr(void *mem) {
    return ((mem_head_t *)mem);
}

mem_block_t *mem_block_ptr(void *mem, uint32_t offset) {
    uint32_t     addr;
    mem_block_t  *ptr;

    addr = (uint32_t)mem + offset;
    ptr  = (mem_block_t *)addr;
    return ptr;
}

uint32_t rtx_mem_init(void *mem, uint32_t size)
{
    mem_head_t  *head;
    mem_block_t *ptr;

    // check parameters
    if ((mem == NULL) || (((uint32_t)mem & 7U) != 0U) || ((size & 7U) != 0U) ||
        (size < (sizeof(mem_head_t) + (2U*sizeof(mem_block_t))))) {
        return 0U;
    }

    // initialize memory pool header
    head = mem_head_ptr(mem);
    head->size = size;
    head->used = sizeof(mem_head_t) + sizeof(mem_block_t);

    // initialize first and last block header
    ptr = mem_block_ptr(mem, sizeof(mem_head_t));
    ptr->next = mem_block_ptr(mem, size - sizeof(mem_block_t));
    ptr->next->next = NULL;
    ptr->next->info = sizeof(mem_head_t) + sizeof(mem_block_t);
    ptr->info = 0U;
    return 1U;
}

void *rtx_mem_alloc(void *mem, uint32_t size, uint32_t type) 
{
    mem_block_t *ptr;
    mem_block_t *p, *p_new;
    uint32_t     block_size;
    uint32_t     hole_size;

    if ((mem == NULL) || (size == 0U) || ((type & ~MB_INFO_TYPE_MASK) != 0U)) {
        return NULL;
    }

    // add block header to size
    block_size = size + sizeof(mem_block_t);
    // make sure that block is 8-byte aligned
    block_size = (block_size + 7U) & ~((uint32_t)7U);

    // search for hole big enough
    p = mem_block_ptr(mem, sizeof(mem_head_t));
    for (;;) {
        hole_size  = (uint32_t)p->next - (uint32_t)p;
        hole_size -= p->info & MB_INFO_LEN_MASK;
        if (hole_size >= block_size) {
            // hole found
            break;
        }
        p = p->next;
        if (p->next == NULL) {
            // failed (end of list)
            return NULL;
        }
    }

    // update used memory
    (mem_head_ptr(mem))->used += block_size;

    // update max used memory
    p_new = mem_block_ptr(mem, (mem_head_ptr(mem))->size - sizeof(mem_block_t));
    if (p_new->info < (mem_head_ptr(mem))->used) {
        p_new->info = (mem_head_ptr(mem))->used;
    }

    // allocate block
    if (p->info == 0U) {
        // No block allocated, set info of first element
        p->info = block_size | type;
        ptr = mem_block_ptr(p, sizeof(mem_block_t));
    } else {
        // insert new element into the list
        p_new = mem_block_ptr(p, p->info & MB_INFO_LEN_MASK);
        p_new->next = p->next;
        p_new->info = block_size | type;
        p->next = p_new;
        ptr = mem_block_ptr(p_new, sizeof(mem_block_t));
    }

    return ptr;
}

uint32_t rtx_mem_free(void *mem, void *block)
{
    const mem_block_t *ptr;
    mem_block_t *p, *p_prev;

    if ((mem == NULL) || (block == NULL)) {
        return 0U;
    }

    // memory block header
    ptr = mem_block_ptr(block, 0U);
    ptr--;

    // search for block header
    p_prev = NULL;
    p = mem_block_ptr(mem, sizeof(mem_head_t));
    while (p != ptr) {
        p_prev = p;
        p = p->next;
        if (p == NULL) {
            // not found
            return 0U;
        }
    }

    // ypdate used memory
    (mem_head_ptr(mem))->used -= p->info & MB_INFO_LEN_MASK;

    // free block
    if (p_prev == NULL) {
        // release first block, only set info to 0
        p->info = 0U;
    } else {
        // discard block from chained list
        p_prev->next = p->next;
    }

    return 1U;
}
