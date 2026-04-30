#include "cm_backtrace.h"
#include "hardfault_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <board_config.h>
#if defined(CONFIG_MODULE_KPRINTF)
#include "kprintf.h"
#endif

#define BACKUPMEM_SIZE  (4*1024)
//fmcsram backupram
__attribute__((section(".backupram"))) static uint8_t membackup[BACKUPMEM_SIZE];
volatile uint8_t *hardfault_cnter = &membackup[0];
volatile uint8_t *hardfault_logger = &membackup[1];
uint16_t hardfault_bx = 0;

void hardfault_log_start()
{
    (*hardfault_cnter)++;
    hardfault_bx = 0;
    memset((void *)hardfault_logger, 0, BACKUPMEM_SIZE - 1);
}

void hardfault_log_end()
{
    for (int i = 0; i < 1000*500; i++);
    board_reboot();
}

void hardfault_log_write(const char *format, ...)
{
    va_list args;
    int max_len  = 0;
    int written = 0;
    if (hardfault_bx >= BACKUPMEM_SIZE - 2) {
        return;
    }

    max_len = BACKUPMEM_SIZE - 2 - hardfault_bx;

    va_start(args, format);
    written = vsnprintf((char *)&hardfault_logger[hardfault_bx], max_len+1, format, args);
    va_end(args);

    if (written > 0) {
        hardfault_bx += (written < max_len ? written : max_len);
    }
}

void hardfault_log_clear()
{
    (*hardfault_cnter) = 0;
    memset((void *)hardfault_logger, 0, BACKUPMEM_SIZE - 1);
}

uint8_t *hardfault_log_check()
{
    if ((*hardfault_cnter) > 0) {
        return (uint8_t *)hardfault_logger;
    }

    return NULL;
}
