#ifndef __LIBXX_LIBXX_HXX
#define __LIBXX_LIBXX_HXX

#include <FreeRTOS.h>
#include <tasks.h>

#include <new>

#define lib_malloc(s)    pvPortMalloc(s)
#define lib_free(p)      vPortFree(p)

#endif // __LIBXX_LIBXX_HXX
