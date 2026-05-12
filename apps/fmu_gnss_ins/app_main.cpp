#include <board_config.h>
#include <drv_hrt.h>
#include <workqueue_manager.h>
#include "uorb_common.h"
#include "uorb_manager.h"

extern "C" void main_root(void *p)
{
    uorb_manager_initialize();
#if defined(BOARD_PXBOARD_EBFV2)
#endif
    vTaskDelete(NULL);
}
