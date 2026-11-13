#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rtxmem.h"
#include "rtklib.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#define RTKLIB_MEMPOOL_TOTAL_SIZE          (1023*1024)
static uint8_t mempool[RTKLIB_MEMPOOL_TOTAL_SIZE];
void *rtkport_malloc(size_t sz) { if (sz == 0) return NULL; return rtx_mem_alloc(mempool, sz, 1); }
void *rtkport_calloc(size_t num, size_t sz) { return rtkport_malloc(num*sz); }
void  rtkport_free(void *p) { rtx_mem_free(mempool, p); }
void *rtkport_realloc(void *p, size_t sz)
{
    void *tmp = rtkport_malloc(sz);
    if (!tmp) {
        return tmp;
    }

    memcpy(tmp, p, sz);
    rtkport_free(p);
    return tmp;
}
#else
void *rtkport_malloc(size_t sz) { return malloc(sz); }
void *rtkport_calloc(size_t num, size_t sz) { return calloc(num,sz); }
void  rtkport_free(void *p) { free(p); }
void *rtkport_realloc(void *p, size_t sz) { return realloc(p, sz); }
#endif

int main(int argc, char **argv)
{
    rtx_mem_init(&mempool[0], RTKLIB_MEMPOOL_TOTAL_SIZE);

    extern int rtklib_main(int argc, char **argv); 
    rtklib_main(0, NULL);

    extern int rtklib_start(int argc, char **argv);
    rtklib_start(0, NULL);

    while(1) {}
    return 0;
}

#ifdef __cplusplus
}
#endif
