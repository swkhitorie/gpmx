
FR_CINCDIRS += include/fr

# CONFIG_FR_TOOLCHAIN = gcc or rvds

ifeq (${CONFIG_FR_ARCH},m7)
FR_PLATFORM := cortex_m7
else ifeq (${CONFIG_FR_ARCH},m4)
FR_PLATFORM := cortex_m4f
else ifeq (${CONFIG_FR_ARCH},m3)
FR_PLATFORM := cortex_m3
else ifeq (${CONFIG_FR_ARCH},m0)
FR_PLATFORM := cortex_m0
endif

FR_CINCDIRS += arch/${FR_PLATFORM}/${CONFIG_FR_TOOLCHAIN}
FR_CSOURCES += arch/${FR_PLATFORM}/${CONFIG_FR_TOOLCHAIN}/port.c

FR_CSOURCES += sched/event_groups.c
FR_CSOURCES += sched/list.c
FR_CSOURCES += sched/queue.c
FR_CSOURCES += sched/stream_buffer.c
FR_CSOURCES += sched/tasks.c
FR_CSOURCES += sched/timers.c
FR_CSOURCES += sched/user.c

ifeq (${CONFIG_FR_MEM_METHOD},1)
FR_CSOURCES += mm/heap_1.c
else ifeq (${CONFIG_FR_MEM_METHOD},2)
FR_CSOURCES += mm/heap_2.c
else ifeq (${CONFIG_FR_MEM_METHOD},3)
FR_CSOURCES += mm/heap_3.c
else ifeq (${CONFIG_FR_MEM_METHOD},4)
FR_CSOURCES += mm/heap_4.c
else ifeq (${CONFIG_FR_MEM_METHOD},5)
FR_CSOURCES += mm/heap_5.c
endif

include ${FR_ROOTDIR}/libs/frtos_libs_defs.mk
FR_CSOURCES += ${addprefix libs/,${FR_LIB_CSOURCES}}

ifeq (${CONFIG_FR_POSIX},y)
FR_CINCDIRS += include
endif

ifeq (${CONFIG_FR_USB},y)
include ${FR_ROOTDIR}/usb/usb.mk
FR_CSOURCES += ${addprefix usb/,${USB_CSOURCES}}
FR_CINCDIRS += ${addprefix usb/,${USB_CINCDIRS}}
endif

