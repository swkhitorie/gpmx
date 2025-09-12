
PROJ_CINCDIRS += kernel/freertos/include

ifeq (${MOD_ARCH},m7)
FR_PLATFORM := cortex_m7
else ifeq (${MOD_ARCH},m4)
FR_PLATFORM := cortex_m4
else ifeq (${MOD_ARCH},m3)
FR_PLATFORM := cortex_m3
else ifeq (${MOD_ARCH},m0)
FR_PLATFORM := cortex_m0
endif

PROJ_CINCDIRS += kernel/freertos/arch/${FR_PLATFORM}/${MK_RTOS_PLATFORM}
CSOURCES += kernel/freertos/arch/${FR_PLATFORM}/${MK_RTOS_PLATFORM}/port.c

ifeq (${MK_RTOS_MEM_METHOD},1)
CSOURCES += kernel/freertos/mm/heap_1.c
else ifeq (${MK_RTOS_MEM_METHOD},2)
CSOURCES += kernel/freertos/mm/heap_2.c
else ifeq (${MK_RTOS_MEM_METHOD},3)
CSOURCES += kernel/freertos/mm/heap_3.c
else ifeq (${MK_RTOS_MEM_METHOD},4)
CSOURCES += kernel/freertos/mm/heap_4.c
else ifeq (${MK_RTOS_MEM_METHOD},5)
CSOURCES += kernel/freertos/mm/heap_5.c
endif

CSOURCES += kernel/freertos/event_groups.c
CSOURCES += kernel/freertos/list.c
CSOURCES += kernel/freertos/queue.c
CSOURCES += kernel/freertos/stream_buffer.c
CSOURCES += kernel/freertos/tasks.c
CSOURCES += kernel/freertos/timers.c
