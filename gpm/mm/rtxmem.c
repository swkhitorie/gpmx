#include "rtxmem.h"

#ifndef NULL
#define NULL (0)
#endif

mem_head_t *mem_head_ptr(void *mem)
{
    return ((mem_head_t *)mem);
}

mem_block_t *mem_block_ptr(void *mem, size_t offset)
{
    return (mem_block_t *)((unsigned char *)mem + offset);
}

size_t rtx_mem_used(void *mem)
{
    mem_head_t *head = mem_head_ptr(mem);
    return head->used;
}

uint32_t rtx_mem_init(void *mem, size_t size)
{
    mem_head_t  *head;
    mem_block_t *ptr;

    /* check parameters */
    if ((mem == NULL) || (((uintptr_t)mem & (uintptr_t)7U) != 0U) ||
        ((size & (size_t)7U) != 0U) ||
        (size < (sizeof(mem_head_t) + 2U * sizeof(mem_block_t))))
    {
        return 0U;
    }

    /* initialize memory pool header */
    head = mem_head_ptr(mem);
    head->size = size;
    head->used = sizeof(mem_head_t) + sizeof(mem_block_t);

    /* initialize first and last block header */
    ptr = mem_block_ptr(mem, sizeof(mem_head_t));
    ptr->next = mem_block_ptr(mem, size - sizeof(mem_block_t));
    ptr->next->next = NULL;
    ptr->next->info = sizeof(mem_head_t) + sizeof(mem_block_t);
    ptr->info = 0U;
    return 1U;
}

void *rtx_mem_alloc(void *mem, size_t size, uint32_t type)
{
    mem_block_t *ptr;
    mem_block_t *p, *p_new;
    size_t       block_size;
    size_t       hole_size;

    if ((mem == NULL) || (size == 0U) ||
        (((size_t)type & ~MB_INFO_TYPE_MASK) != 0U)) {
        return NULL;
    }

    /* overflow protect */
    if (size > (SIZE_MAX - sizeof(mem_block_t) - (size_t)7U)) {
        return NULL;
    }

    /* add block header to size */
    block_size = size + sizeof(mem_block_t);
    block_size = (block_size + (size_t)7U) & ~(size_t)7U;

    /* search for hole big enough */
    p = mem_block_ptr(mem, sizeof(mem_head_t));
    for (;;) {
        hole_size  = (size_t)((unsigned char *)p->next - (unsigned char *)p);
        hole_size -= p->info & MB_INFO_LEN_MASK;
        if (hole_size >= block_size) {
            break;
        }
        p = p->next;
        if (p->next == NULL) {
            return NULL;
        }
    }

    /* update used memory */
    mem_head_ptr(mem)->used += block_size;

    /* update max used memory */
    p_new = mem_block_ptr(mem, mem_head_ptr(mem)->size - sizeof(mem_block_t));
    if (p_new->info < mem_head_ptr(mem)->used) {
        p_new->info = mem_head_ptr(mem)->used;
    }

    /* allocate block */
    if (p->info == 0U) {
        p->info = block_size | (size_t)type;
        ptr = mem_block_ptr(p, sizeof(mem_block_t));
    } else {
        p_new = mem_block_ptr(p, p->info & MB_INFO_LEN_MASK);
        p_new->next = p->next;
        p_new->info = block_size | (size_t)type;
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

    ptr = (const mem_block_t *)block - 1;

    /* search for block header */
    p_prev = NULL;
    p = mem_block_ptr(mem, sizeof(mem_head_t));
    while (p != ptr) {
        p_prev = p;
        p = p->next;
        if (p == NULL) {
            return 0U;
        }
    }

    /* update used memory */
    mem_head_ptr(mem)->used -= p->info & MB_INFO_LEN_MASK;

    /* free block */
    if (p_prev == NULL) {
        p->info = 0U;
    } else {
        p_prev->next = p->next;
    }

    return 1U;
}
