
#include "libxx.hxx"

void operator delete(FAR void *ptr, unsigned int size)
{
    lib_free(ptr);
}
