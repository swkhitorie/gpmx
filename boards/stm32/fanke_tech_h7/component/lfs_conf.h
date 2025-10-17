#ifndef LITTLEFS_CONFIG_H_
#define LITTLEFS_CONFIG_H_

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <tasks.h>
#define LFS_MALLOC(sz) pvPortMalloc(sz)
#define LFS_FREE(p) vPortFree(p)
#endif

#endif
