
ifeq (${CONFIG_FREERTOS_ENABLE},y)

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

ifeq (${CONFIG_RTOS_COMPILER},"gcc")
PROJ_CINCDIRS += ${GPMPATH}/sched/freertos/arch/${FR_PLATFORM}/gcc
CSOURCES += ${GPMPATH}/sched/freertos/arch/${FR_PLATFORM}/gcc/port.c
else ifeq (${CONFIG_RTOS_COMPILER},"rvds")
PROJ_CINCDIRS += ${GPMPATH}/sched/freertos/arch/${FR_PLATFORM}/rvds
CSOURCES += ${GPMPATH}/sched/freertos/arch/${FR_PLATFORM}/rvds/port.c
endif

ifeq (${CONFIG_RTOS_MEM_METHOD},1)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_1.c
else ifeq (${CONFIG_RTOS_MEM_METHOD},2)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_2.c
else ifeq (${CONFIG_RTOS_MEM_METHOD},3)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_3.c
else ifeq (${CONFIG_RTOS_MEM_METHOD},4)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_4.c
else ifeq (${CONFIG_RTOS_MEM_METHOD},5)
CSOURCES += ${GPMPATH}/sched/freertos/mm/heap_5.c
endif

CSOURCES += ${GPMPATH}/sched/freertos/event_groups.c
CSOURCES += ${GPMPATH}/sched/freertos/list.c
CSOURCES += ${GPMPATH}/sched/freertos/queue.c
CSOURCES += ${GPMPATH}/sched/freertos/stream_buffer.c
CSOURCES += ${GPMPATH}/sched/freertos/tasks.c
CSOURCES += ${GPMPATH}/sched/freertos/timers.c

ifeq (${CONFIG_GMSH},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/freertos/gmsh
CSOURCES += ${GPMPATH}/sched/freertos/gmsh/msh.c
CSOURCES += ${GPMPATH}/sched/freertos/gmsh/shell.c
CSOURCES += ${GPMPATH}/sched/freertos/gmsh/cmd.c
endif

include ${GPMPATH}/sched/freertos/test/config.mk

endif
