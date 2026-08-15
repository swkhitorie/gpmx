
#include <gpmx/config.h>
#include <mlog.h>

#if defined(CONFIG_FREERTOS_ENABLE)
int (*mlog_printf)(const char *format, ...) = gsh_kprintf;
#elif defined(CONFIG_RTTNANO_ENABLE)
int (*mlog_printf)(const char *format, ...) = rt_kprintf;
#elif defined(CONFIG_POSIXRUN_ENABLE)
#include <stdio.h>
int (*mlog_printf)(const char *format, ...) = printf;
#else
int (*mlog_printf)(const char *format, ...) = ((void *)0);
#endif

void mlog_redirect(int (*interface_log)(const char *format, ...))
{
    mlog_printf = interface_log;
}
