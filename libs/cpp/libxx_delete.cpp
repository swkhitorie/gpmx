
#include "libxx.hpp"

void operator delete(void* ptr)
{
    lib_free(ptr);
    // ptr = nullptr;
}
