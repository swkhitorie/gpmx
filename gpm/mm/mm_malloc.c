#include <stdlib.h>
#include <unistd.h>
#include "gpm/sched.h"

#undef malloc
void *malloc(size_t size)
{
    return kmm_malloc(size);
}
