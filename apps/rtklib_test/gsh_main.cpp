
#include <stdio.h>
#include <stdarg.h>
#include <board_config.h>
#include "gmsh.h"
#include "device/dnode.h"

#ifdef __cplusplus
extern "C" {
#endif

char gmsh_console_getchar()
{
    char c = (char)(-1);
    int ret = board_stream_in(0, &c, 1);
    if (ret != 1) {
#if defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_mdelay(10);
#elif defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(pdMS_TO_TICKS(10));
#endif
    }
    return c;
}

void gsh_kprintf(const char *format, ...)
{
    char gsh_buf[512] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(gsh_buf, sizeof(gsh_buf), format, args);
    va_end(args);
    board_printf("%s", gsh_buf);
}

#ifdef __cplusplus
}
#endif
