#ifndef __LIBXX_LIBXX_HXX
#define __LIBXX_LIBXX_HXX

#include <new>

#include <FreeRTOS.h>
#include <portable.h>
#define lib_malloc(s)    pvPortMalloc(s)
#define lib_free(p)      vPortFree(p)

#define sys_mem_malloc(s)   lib_malloc(s)
#define sys_mem_free(p)     lib_free(p)

#ifdef __cplusplus
extern "C" {
#endif

void *sys_mem_realloc(void *p, size_t old_sz, size_t new_sz);

#ifdef __cplusplus
}
#endif

#endif // __LIBXX_LIBXX_HXX
