
#include <gpmx/config.h>
#include <stdlib.h>
#include <driver/drv_sched.h>

#if !defined(CONFIG_MALLOC_SCHED_IMPLEMENT_DISABLE)
#undef malloc
void *malloc(size_t size)
{

    return kmm_malloc(size);
}
#endif
