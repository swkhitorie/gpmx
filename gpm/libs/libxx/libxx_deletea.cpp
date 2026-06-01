
#include "libxx.hpp"

void operator delete[](void *ptr) noexcept
{
    lib_free(ptr);
    // ptr = nullptr;
}
