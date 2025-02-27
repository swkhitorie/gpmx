
#include "libxx.hxx"

void operator delete[](void *ptr)
{
    lib_free(ptr);
}
