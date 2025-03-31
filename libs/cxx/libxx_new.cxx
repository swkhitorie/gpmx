
#include "libxx.hxx"

void *operator new(unsigned int nbytes)
{
    if (nbytes < 1) {
        nbytes = 1;
    }

    void *alloc = lib_malloc(nbytes);
    return alloc;
}

void *sys_mem_realloc(void *p, size_t old_sz, size_t new_sz)
{
    sys_mem_free(p);
    return sys_mem_malloc(new_sz);
}

