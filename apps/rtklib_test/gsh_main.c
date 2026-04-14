#include "gmsh.h"
#include <board_config.h>
#include <stdio.h>
#include <stdarg.h>

#include <FreeRTOS.h>
#include <task.h>

char gmsh_console_getchar()
{
    char c = (char)(-1);
    int ret = board_stream_in(0, &c, 1);
    if (ret != 1) vTaskDelay(10);
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

