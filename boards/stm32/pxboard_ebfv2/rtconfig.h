/* RT-Thread config file */

#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

/****************************************************************************
 * Basic Configuration
 ****************************************************************************/
// Maximal level of thread priority <8-256>, Default: 32
#define RT_THREAD_PRIORITY_MAX  32

// OS tick per second, Default: 1000   (1ms)
#define RT_TICK_PER_SECOND  1000

// Alignment size for CPU architecture data access, Default: 4
#define RT_ALIGN_SIZE   4

// the max length of object name<2-16>, Default: 8
#define RT_NAME_MAX    8

// Using RT-Thread components initialization
#define RT_USING_COMPONENTS_INIT

#define RT_USING_USER_MAIN

// the stack size of main thread<1-4086>, Default: 512
#define RT_MAIN_THREAD_STACK_SIZE     256

/****************************************************************************
 * Debug Configuration
 ****************************************************************************/
// <c1>enable kernel debug configuration
//  <i>Default: enable kernel debug configuration
//#define RT_DEBUG
// </c>

// <o>enable components initialization debug configuration<0-1>
//  <i>Default: 0
#define RT_DEBUG_INIT 0
// <c1>thread stack over flow detect
//  <i> Diable Thread stack over flow detect
//#define RT_USING_OVERFLOW_CHECK
// </c>

/****************************************************************************
 * Hook Configuration
 ****************************************************************************/
// using hook
//#define RT_USING_HOOK

// using idle hook
//#define RT_USING_IDLE_HOOK


/****************************************************************************
 * Software timers Configuration
 ****************************************************************************/
// Enables user timers
#define RT_USING_TIMER_SOFT         0
#if RT_USING_TIMER_SOFT == 0
    #undef RT_USING_TIMER_SOFT
#endif

// The priority level of timer thread <0-31>, Default: 4
#define RT_TIMER_THREAD_PRIO        4

// The stack size of timer thread <0-8192>, Default: 512
#define RT_TIMER_THREAD_STACK_SIZE  512

/****************************************************************************
 * IPC(Inter-process communication) Configuration
 ****************************************************************************/
// Using Semaphore
#define RT_USING_SEMAPHORE

// Using Mutex
#define RT_USING_MUTEX

// Using Event
//#define RT_USING_EVENT

// Using Signals
//#define RT_USING_SIGNALS

// Using MailBox 
#define RT_USING_MAILBOX

// Using Message Queue
//#define RT_USING_MESSAGEQUEUE

/****************************************************************************
 * Memory Management Configuration
 ****************************************************************************/
// Memory Pool Management
//#define RT_USING_MEMPOOL

// Dynamic Heap Management(Algorithm: small memory )
#define RT_USING_HEAP
#define RT_USING_SMALL_MEM
#define RT_USING_SMALL_MEM_AS_HEAP

// using tiny size of memory
//#define RT_USING_TINY_SIZE

/****************************************************************************
 * Console Configuration
 ****************************************************************************/
// Using console
//#define RT_USING_CONSOLE

// the buffer size of console <1-1024>, Default: 128  (128Byte)
#define RT_CONSOLEBUF_SIZE          256

/****************************************************************************
 * FinSH Configuration
 ****************************************************************************/
// include finsh config
//#include "finsh_config.h"

/****************************************************************************
 * Device Configuration
 ****************************************************************************/
// using device framework
//#define RT_USING_DEVICE

#endif
