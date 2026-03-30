#include <board_config.h>

void main_root(void *p)
{
    hrt_init();
    workqueue_manager_start();

#if defined(BOARD_PXBOARD_EBFV2)

#endif

}
