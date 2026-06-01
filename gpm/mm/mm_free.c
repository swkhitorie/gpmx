#include <stdlib.h>
#include "gpm/sched.h"

#undef free
void free(void* ptr)
{
    kmm_free(ptr);
}
