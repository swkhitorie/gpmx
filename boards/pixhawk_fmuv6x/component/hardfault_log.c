#if defined(CONFIG_MODULE_CMBACKTRACE)

#include "cm_backtrace.h"
#include "hardfault_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <board_config.h>
#if defined(CONFIG_MODULE_KPRINTF)
#include "kprintf.h"
#endif


void hardfault_log_start()
{

}

void hardfault_log_end()
{

}

void hardfault_log_write(const char *format, ...)
{

}

void hardfault_log_clear()
{

}

uint8_t *hardfault_log_check()
{

    return NULL;
}

#endif
