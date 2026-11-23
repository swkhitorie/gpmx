#include <gpmx/config.h>
#include <stdlib.h>
#include <drivers/drv_sched.h>

#if !defined(CONFIG_MALLOC_SCHED_IMPLEMENT_DISABLE)
#undef free
void free(void* ptr)
{
    kmm_free(ptr);
}
#endif
