
PROJ_CINCDIRS += ${GPMPATH}/sched/freertos/include

ifeq (${MOD_ARCH},m7)
FR_PLATFORM := cortex_m7
else ifeq (${MOD_ARCH},m4)
FR_PLATFORM := cortex_m4
else ifeq (${MOD_ARCH},m3)
FR_PLATFORM := cortex_m3
else ifeq (${MOD_ARCH},m0)
FR_PLATFORM := cortex_m0
endif

PROJ_CINCDIRS += ${GPMPATH}/sched/freertos/arch/${FR_PLATFORM}/${MK_RTOS_PLATFORM}
CSOURCES += ${GPMPATH}/sched/freertos/arch/${FR_PLATFORM}/${MK_RTOS_PLATFORM}/port.c

ifeq (${MK_RTOS_MEM_METHOD},1)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_1.c
else ifeq (${MK_RTOS_MEM_METHOD},2)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_2.c
else ifeq (${MK_RTOS_MEM_METHOD},3)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_3.c
else ifeq (${MK_RTOS_MEM_METHOD},4)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_4.c
else ifeq (${MK_RTOS_MEM_METHOD},5)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_5.c
endif

CSOURCES += ${GPMPATH}/sched/freertos/event_groups.c
CSOURCES += ${GPMPATH}/sched/freertos/list.c
CSOURCES += ${GPMPATH}/sched/freertos/queue.c
CSOURCES += ${GPMPATH}/sched/freertos/stream_buffer.c
CSOURCES += ${GPMPATH}/sched/freertos/tasks.c
CSOURCES += ${GPMPATH}/sched/freertos/timers.c

