#ifndef RTKLIB_CONFIG_H_
#define RTKLIB_CONFIG_H_

#if defined(CONFIG_FREERTOS_ENABLE)

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define thread_t    void *
#define lock_t      SemaphoreHandle_t
#define initlock(f) f = xSemaphoreCreateMutex()
#define lock(f)     xSemaphoreTake(f, 5000)
#define unlock(f)   xSemaphoreGive(f)
#define FILEPATHSEP '\\'
#else
#error please select a rtos
#define thread_t    void *
#define lock_t      int
#define initlock(f) f = 0;
#define lock(f)
#define unlock(f)
#define FILEPATHSEP '\\'
#endif

#endif

