#include <stdio.h>
#include <stdarg.h>
#include <driver/drv_sched.h>
#if defined(CONFIG_KPRINTF)
#include "kprintf.h"
#endif

int vasprintf(char **strp, const char *fmt, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0) { 
        *strp = NULL;
        return -1; 
    }

    char *buf = kmm_malloc((size_t)len + 1);

    if (!buf) { 
        *strp = NULL;
        return -1; 
    }

    vsnprintf(buf, (size_t)len + 1, fmt, ap);
    *strp = buf;
    return len;
}
