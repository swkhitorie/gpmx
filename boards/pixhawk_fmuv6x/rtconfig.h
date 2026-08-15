/* RT-Thread config file */

#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

#define RT_VER_NUM 0x40101

/******* Basical Configuration */
#define RT_ALIGN_SIZE   8
#define RT_NAME_MAX    16
#define RT_TICK_PER_SECOND 1000

// #define RT_USING_SMP
// #define ARCH_CPU_64BIT
// #define ARCH_CPU_STACK_GROWS_UPWARD
// #define RT_USING_ARCH_DATA_TYPE
// #define RT_CPU_CACHE_LINE_SZ    32
// #define RT_USING_INTERRUPT_INFO
// #define RT_USING_LIBC
#define RT_USING_CPU_FFS
// #define RT_USING_TINY_FFS

#define RT_THREAD_PRIORITY_MAX  32
#define RT_USING_PTHREADS

#define RT_USING_CPU_USAGE
#if defined(RT_USING_CPU_USAGE)
#include "gpm/drv_hrt.h"
#define RT_CPU_USAGE_HRT_VALUE hrt_absolute_time
#endif

// #define RT_USING_MODULE
// #define RT_USING_LWP

#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE   2048
#define RT_MAIN_THREAD_PRIORITY     10
#define RT_USING_COMPONENTS_INIT

/******* Debug Configuration */
#define RT_USING_OVERFLOW_CHECK

#define RT_USING_HOOK
#define RT_HOOK_USING_FUNC_PTR
#define RT_USING_IDLE_HOOK
// #define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 1024
// #define RT_USING_HEAP_ISR

// #define RT_DEBUG_INIT
// #define RT_DEBUG
// #define RT_DEBUG_MEM       0
// #define RT_DEBUG_MEMHEAP   0
// #define RT_DEBUG_MODULE    0
// #define RT_DEBUG_SCHEDULER 0
// #define RT_DEBUG_SLAB      0
// #define RT_DEBUG_THREAD    0
// #define RT_DEBUG_TIMER     0
// #define RT_DEBUG_IRQ       0
// #define RT_DEBUG_IPC       0
// #define RT_DEBUG_DEVICE    0
// #define RT_DEBUG_INIT      0
// #define RT_DEBUG_CONTEXT_CHECK 0
// #define DBG_ENABLE
// #define RT_DEBUG_COLOR
// #define RT_USING_ULOG


/******* Timer Configuration */
// #define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO        4
#define RT_TIMER_THREAD_STACK_SIZE  2048



/******* Console Configuration */
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE          256



/******* IPC(Inter-process communication) Configuration */
#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_SIGNALS
// #define RT_SIG_INFO_MAX 32
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE



/******* Memory Manage Configuration */
#define RT_USING_MEMPOOL
#define RT_USING_HEAP
// #define RT_USING_USERHEAP
// #define RT_USING_MEMTRACE

#define RT_USING_SMALL_MEM
#define RT_USING_SMALL_MEM_AS_HEAP
// #define RT_USING_MEMHEAP
// #define RT_USING_MEMHEAP_AS_HEAP
// #define RT_USING_MEMHEAP_AUTO_BINDING
// #define RT_USING_SLAB
// #define RT_USING_SLAB_AS_HEAP
// #define RT_DEBUG_SLAB
// #define RT_MEMHEAP_BSET_MODE



/******* FINSH Configuration */
#define RT_USING_FINSH
#define FINSH_USING_DESCRIPTION
#define FINSH_USING_SYMTAB
#define FINSH_THREAD_NAME "tshell"
#define FINSH_THREAD_PRIORITY   16
#define FINSH_THREAD_STACK_SIZE  2048
#define FINSH_CMD_SIZE   80
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
// #define FINSH_USING_AUTH
// #define FINSH_PASSWORD_MAX  16
// #define FINSH_PASSWORD_MIN  6
// #define FINSH_DEFAULT_PASSWORD  "rtthread"
#define MSH_USING_BUILT_IN_COMMANDS


/******* Other Configuration */
// #define RT_USING_DEVICE
// #define RT_USING_DEVICE_OPS
// #define RT_USING_POSIX_STDIO
// #define DFS_USING_POSIX
// #define DFS_USING_WORKDIR
// #define RT_USING_DFS




#endif
