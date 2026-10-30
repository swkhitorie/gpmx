/*
*   @file  dl_port.c
*   @author  wzh
*   @version 1.0
*   @date 2021-12-3
*   @description 与硬件相关的底层函数接口
*   @detail	此处的函数原型需要自己提供
*/
#include "dl_port.h"
#include "ff.h"
#include <stdlib.h>

#if 1
#include "rtxmem.h"
static uint8_t mem_dl_pool[CONFIG_DLOADER_MEM_SIZE] __attribute__((aligned(8)));
#endif

void dl_port_init()
{
#if 1
	rtx_mem_init(mem_dl_pool, CONFIG_DLOADER_MEM_SIZE);
#endif
}

void* dl_port_aligned_alloc(size_t align_byte, size_t size)
{
#if 1
    void     *raw;
    void     *aligned;
    uintptr_t base;
    uint32_t  total;

    if (align_byte == 0 || (align_byte & (align_byte - 1)) != 0) {
        return NULL;
    }

    if (align_byte < sizeof(void *)) {
        align_byte = sizeof(void *);
    }

    if (size == 0) {
        return NULL;
    }

    /* total = size + (align_byte - 1) + sizeof(void*) */
    if (size > (SIZE_MAX - align_byte - sizeof(void *) + 1U)) {
        return NULL;
    }

    total = (uint32_t)(size + align_byte - 1U + sizeof(void *));

    raw = rtx_mem_alloc(mem_dl_pool, total, 0U);
    if (raw == NULL) {
        return NULL;
    }

    base    = (uintptr_t)raw + (uintptr_t)sizeof(void *);
    aligned = (void *)((base + (uintptr_t)align_byte - 1U)
                        & ~((uintptr_t)align_byte - 1U));

    ((void **)aligned)[-1] = raw;

    return aligned;
#else
    extern void *aligned_alloc(size_t align_byte, size_t size);
    return aligned_alloc(align_byte,size);
#endif
}

void dl_port_aligned_free(void* ptr)
{
#if 1
    rtx_mem_free(&mem_dl_pool, ptr);
#else
    free(ptr);
#endif
}

void* dl_port_malloc(size_t size)
{
#if 1
    return rtx_mem_alloc(&mem_dl_pool, size, 0);
#else
    return malloc(size);
#endif
}

void dl_port_free(void* ptr)
{
#if 1
    rtx_mem_free(&mem_dl_pool, ptr);
#else
    free(ptr);
#endif
}

void* dl_port_open_file(const char* file_name)
{
    FIL* pfile;

#if 1
    pfile = rtx_mem_alloc(&mem_dl_pool, sizeof(FIL), 0);
#else
    pfile = malloc(sizeof(FIL));
#endif
    if (pfile == NULL) {
        return NULL;
    }

    if (f_open(pfile, file_name, FA_READ) != FR_OK) {
#if 1
        rtx_mem_free(&mem_dl_pool, pfile);
#else
        free(pfile);
#endif
        return NULL;
    }

    return (void*)pfile;
}

size_t dl_port_get_file_size(void* file_handle)
{
    return f_size((FIL*)file_handle);
}

bool dl_port_read_file(void* file_handle,void* buf,size_t len)
{
    UINT br;
    return (f_read(file_handle,buf,len,&br) == FR_OK) && (len == br);
}

void dl_port_close_file(void* file_handle)
{
    f_close(file_handle);
#if 1
    rtx_mem_free(&mem_dl_pool, file_handle);
#else
    free(file_handle);
#endif
}

#if defined(DL_CACHE_USE) && (DL_CACHE_USE == 1)
// invalid I_CACHE
#include "stm32h7xx.h"
void dl_port_cache_process(void* addr,size_t size)
{
    SCB_CleanDCache_by_Addr(addr,size);
    SCB_InvalidateICache_by_Addr(addr,size);
}
#endif
