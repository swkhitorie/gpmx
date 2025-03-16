
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* FreeRTOS POSIX Cannot support create task dynamic */
extern uint32_t SystemCoreClock;
#define vAssertCalled(char,int) //board_blue_led_toggle()
#define configASSERT(x) if((x)==0) vAssertCalled(__FILE__,__LINE__)

/****************************************************************************
 * FreeRTOS Interrupt Configuration
 ****************************************************************************/
#define xPortPendSVHandler 	PendSV_Handler
#define vPortSVCHandler 	SVC_Handler
// #define xPortSysTickHandler SysTick_Handler

#ifdef __NVIC_PRIO_BITS
	#define configPRIO_BITS       		__NVIC_PRIO_BITS
#else
	#define configPRIO_BITS       		4                  
#endif
#define configUSE_PORT_OPTIMISED_TASK_SELECTION         1     /* 1: Enable special way to select next task,  CLZ: Counting leading zero */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			15    /* Interrupt Min Priority */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	9     /* Interrupt Max Priority can be managed */
#define configKERNEL_INTERRUPT_PRIORITY      \
            ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	\
            ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


/****************************************************************************
 * FreeRTOS Basical Configuration
 ****************************************************************************/
#define configTOTAL_HEAP_SIZE					((size_t)(64*1024))     /* total heap size, unit in byte */
#define configCPU_CLOCK_HZ						(SystemCoreClock)             /* CPU Frequence */
#define configTICK_RATE_HZ						(1000)                  /* RTOS Kernel timer Frequence */
#define configMAX_PRIORITIES					(32)                    /* Task Max Priority */
#define configMINIMAL_STACK_SIZE				((unsigned short)130)   /* Idle Task Stack Size, unit in word */
#define configMAX_TASK_NAME_LEN					(16)                    /* Task NameString Max Len */

#define configGENERATE_RUN_TIME_STATS	        0                       /* 1: Enable Task cpu utilization analyze */
#define configSUPPORT_DYNAMIC_ALLOCATION        1                       /* 1: support dynamic allocation */
#define configSUPPORT_STATIC_ALLOCATION         1                       /* 1: support static allocation */

#define configUSE_16_BIT_TICKS					0                       /* 1: System tick type 16bits, 0: 32bits */
#define configUSE_TICKLESS_IDLE					0                       /* 1: Enable Tickless low consumption mode */
#define configUSE_TIME_SLICING					1						/* 1: Enable Time Slice Schedule */
#define configUSE_TASK_NOTIFICATIONS            1                       /* 1: Enable Task Notify Function */
#define configUSE_MUTEXES						1                       /* 1: Enable Mutex Semaphore */
#define configUSE_RECURSIVE_MUTEXES				1                       /* 1: Enable Recursive Mutex Semaphore */
#define configUSE_COUNTING_SEMAPHORES			1                       /* 1: Enable Counting Mutex Semaphore */
#define configUSE_QUEUE_SETS					1                       /* 1: Enable message queue */
#define configUSE_PREEMPTION					1                       /* 1: preemption core, 0: croutine core */
#define configUSE_MALLOC_FAILED_HOOK			1                       /* 1: Enable Malloc Failed Hook */
#define configUSE_IDLE_HOOK						0                       /* 1: Enable Idle Task Hook */
#define configUSE_TICK_HOOK						0                       /* 1: Enable Tick interrupt Hook */
#define configUSE_APPLICATION_TASK_TAG			1 
#define configUSE_TIMERS				        1                       /* 1: Enable Soft Timer */
#define configUSE_CO_ROUTINES 			        0                       /* 1: Enable Croutine, default 0 */
#define configUSE_TRACE_FACILITY				1                       /* 1: Enable Visible Debug */
#define configUSE_STATS_FORMATTING_FUNCTIONS	1                       /* 1: with USE_TRACE_FACILITY 1, Enbale some debug functions */

#define configTIMER_TASK_PRIORITY		        (configMAX_PRIORITIES-1)        /* Soft Timer Priority */
#define configTIMER_QUEUE_LENGTH		        5                               /* Soft Timer Queue length */
#define configTIMER_TASK_STACK_DEPTH	        (configMINIMAL_STACK_SIZE*2)    /* Soft Timer Task Stack depth, unit in word */
#define configMAX_CO_ROUTINE_PRIORITIES         2                               /* Croutine Valid Priority num */
#define configIDLE_SHOULD_YIELD					1     /* if 1, Idle Task will give cpu up to other task having same priority */
#define configQUEUE_REGISTRY_SIZE				8     /* if not 0, value can indicate queue and sempahore recorded num */
#define configCHECK_FOR_STACK_OVERFLOW			0     /* if bigger than 0, user add a stack overflow detection hook function, 1 and 2 means two detection method */

/****************************************************************************
 * FreeRTOS Function Link, Recommand 1, Linker will add or delete automatically
 ****************************************************************************/
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_vTaskPrioritySet		        1
#define INCLUDE_uxTaskPriorityGet		        1
#define INCLUDE_vTaskDelete				        1
#define INCLUDE_vTaskCleanUpResources	        1
#define INCLUDE_vTaskSuspend			        1
#define INCLUDE_vTaskDelayUntil			        1
#define INCLUDE_vTaskDelay				        1
#define INCLUDE_eTaskGetState			        1
#define INCLUDE_xTimerPendFunctionCall	        1
#define INCLUDE_xSemaphoreGetMutexHolder        1

#endif /* FREERTOS_CONFIG_H */
