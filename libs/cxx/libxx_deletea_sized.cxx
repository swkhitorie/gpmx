
#include "libxx.hxx"

void operator delete[](void *ptr, unsigned int size)
{
    lib_free(ptr);
}
