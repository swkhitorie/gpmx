#include <board_config.h>
#include <drv_hrt.h>
#include <workqueue_manager.h>
#include "device/dnode.h"

extern "C" void main_root(void *p)
{
#if defined(BOARD_PXBOARD_EBFV2)
#endif
    vTaskDelete(NULL);
}
