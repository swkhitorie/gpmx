
ifeq (${CONFIG_MK_CHIP_ARCH},m7)
FR_PLATFORM := cortex_m7
else ifeq (${CONFIG_MK_CHIP_ARCH},m4)
FR_PLATFORM := cortex_m4f
else ifeq (${CONFIG_MK_CHIP_ARCH},m3)
FR_PLATFORM := cortex_m3
else ifeq (${CONFIG_MK_CHIP_ARCH},m0)
FR_PLATFORM := cortex_m0
endif

ifeq (${CONFIG_MK_USE_FREERTOS},y)
FR_CINCDIRS += sched/arch/${FR_PLATFORM}/${CONFIG_MK_COMPILER}
FR_CSOURCES += sched/arch/${FR_PLATFORM}/${CONFIG_MK_COMPILER}/port.c

FR_CSOURCES += sched/event_groups.c
FR_CSOURCES += sched/list.c
FR_CSOURCES += sched/queue.c
FR_CSOURCES += sched/stream_buffer.c
FR_CSOURCES += sched/tasks.c
FR_CSOURCES += sched/timers.c
FR_CSOURCES += sched/user.c
endif
