#include "heap_trace.h"

#ifndef CONFIG_MMU_TRACER_BLOCK_LEN
#define CONFIG_MMU_TRACER_BLOCK_LEN (100)
#endif

typedef struct __mmu_tracer
{
    void *block_addr;
    uint32_t block_len;
} mmu_tracer_t;

static mmu_tracer_t mmu_block_list[CONFIG_MMU_TRACER_BLOCK_LEN];
static uint32_t mmu_block_list_size;

static void set_trace(mmu_tracer_t *p, void *addr, uint32_t len)
{
	p->block_addr = addr;
	p->block_len = len;
}

static void swap_node(mmu_tracer_t *a, mmu_tracer_t *b)
{
    mmu_tracer_t tmp;
    set_trace(&tmp, a->block_addr, a->block_len);
    set_trace(a, b->block_addr, b->block_len);
    set_trace(b, tmp.block_addr, tmp.block_len);
}

void mmu_tracerecord(void *addr, uint32_t len)
{
    if (mmu_block_list_size >= MMU_TRACER_BLOCK_LEN) return;
    mmu_block_list[mmu_block_list_size].block_addr = addr;
    mmu_block_list[mmu_block_list_size].block_len = len;
    mmu_block_list_size++;
}

void mmu_traceremove(void *addr)
{
    int i = 0;

    for (; i < mmu_block_list_size; i++) {
        if (addr == mmu_block_list[i].block_addr) {
            set_trace(&mmu_block_list[i], 0, 0);
            break;
        }
    }
    if (i == mmu_block_list_size) return;
    for (; i < mmu_block_list_size; i++) {
        if (i == (mmu_block_list_size - 1))
            break;
        swap_node(&mmu_block_list[i], &mmu_block_list[i + 1]);
    }
    mmu_block_list_size--;
}

void mmu_traceinfo(uint16_t idx, void **addr, uint32_t *len)
{
        *addr = mmu_block_list[idx].block_addr;
        *len  = mmu_block_list[idx].block_len;
}
