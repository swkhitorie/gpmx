#ifndef INCLUDE_GPM_SCHED_H
#define INCLUDE_GPM_SCHED_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "gpm/fs/fs.h"

#if defined (__CC_ARM)
#include "cmsis_armcc.h"
#elif defined(__ARMCC_VERSION)
#include "cmsis_armclang.h"
#elif defined(__GNUC__)
// #include "cmsis_gcc.h"
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <portmacro.h>
#elif defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>
#endif

#if defined(CONFIG_FREERTOS_ENABLE)
#define kmm_malloc(p)                       pvPortMalloc(p)
#define kmm_free(p)                         vPortFree(p)
#define enter_critical_section()            ulPortRaiseBASEPRI()
#define leave_critical_section(x)           vPortSetBASEPRI(x)
#define up_interrupt_context()              (xPortIsInsideInterrupt() == pdTRUE)
#define up_kerneltime()                     (xTaskGetTickCount()*(1000/configTICK_RATE_HZ))
#elif defined(CONFIG_RTTNANO_ENABLE)
#define kmm_malloc(p)                       rt_malloc(p)
#define kmm_free(p)                         rt_free(p)
#define enter_critical_section()            rt_hw_interrupt_disable()
#define leave_critical_section(x)           rt_hw_interrupt_enable(x)
#define up_interrupt_context()              (rt_interrupt_get_nest() > 0)
#define up_kerneltime()                     rt_tick_get_millisecond()
#else
#define kmm_malloc(p)                       (NULL)
#define kmm_free(p)
#define enter_critical_section()            (0)
#define leave_critical_section(x)           
#define up_interrupt_context()              (0)
#define up_kerneltime()                     (0)
#endif

#define GOK       (0)
#define GERROR    (-1)

#if defined(__cplusplus)
extern "C"{
#endif

void *kmm_realloc(void *oldmem, size_t newsize);
void *kmm_zalloc(size_t size);

struct filelist *sched_get_files(void);

bool     up_register(const char *path, void *dev);
void    *up_bind(const char *path);
void    *up_memcpy(void *dst, const void *src, size_t n);

#if defined(__cplusplus)
}
#endif

#endif
