
#include "libxx.hpp"

void operator delete[](void *ptr, unsigned int size)
{
    lib_free(ptr);
    // ptr = nullptr;
}
