
ifeq (${MK_CHIP_ARCH},m7)
FR_PLATFORM := cortex_m7
else ifeq (${MK_CHIP_ARCH},m4)
FR_PLATFORM := cortex_m4f
else ifeq (${MK_CHIP_ARCH},m3)
FR_PLATFORM := cortex_m3
else ifeq (${MK_CHIP_ARCH},m0)
FR_PLATFORM := cortex_m0
endif

ifeq (${MK_USE_FREERTOS},y)
PROJ_CINCDIRS += sched/arch/${FR_PLATFORM}/${MK_COMPILER}
CSOURCES += sched/arch/${FR_PLATFORM}/${MK_COMPILER}/port.c

CSOURCES += sched/event_groups.c
CSOURCES += sched/list.c
CSOURCES += sched/queue.c
CSOURCES += sched/stream_buffer.c
CSOURCES += sched/tasks.c
CSOURCES += sched/timers.c

CSOURCES += sched/fr_hook.c
endif
