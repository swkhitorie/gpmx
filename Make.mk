
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
FR_CSOURCES += ${FR_LIB_CSOURCES}
FR_CSOURCES += ${FR_LIB_INCDIRS}

ifeq (${CONFIG_FR_POSIX},y)
FR_CINCDIRS += include
endif

ifeq (${CONFIG_FR_USB},y)
include ${FR_ROOTDIR}/usb/usb.mk
FR_CSOURCES += ${addprefix usb/,${USB_CSOURCES}}
FR_CINCDIRS += ${addprefix usb/,${USB_CINCDIRS}}
endif

ifeq (${CONFIG_FR_PX4_SUPPORT},y)
FR_CSOURCES += px4/stm/hrt.c
FR_CPPSOURCES += px4/ptasks.cpp
endif

ifeq (${CONFIG_FR_FATFS},y)
FR_CINCDIRS += fs/fat
FR_CSOURCES += fs/fat/ff.c
FR_CSOURCES += fs/fat/diskio.c
FR_CSOURCES += fs/fat/ff_drv.c
FR_CSOURCES += fs/fat/ffsystem.c
FR_CSOURCES += fs/fat/ffunicode.c
endif

ifeq (${CONFIG_FR_FATFS},l)
FR_CINCDIRS += fs/fat/legacy
FR_CSOURCES += fs/fat/legacy/ff.c
FR_CSOURCES += fs/fat/legacy/diskio.c
FR_CSOURCES += fs/fat/legacy/ff_drv.c
FR_CSOURCES += fs/fat/legacy/option/syscall.c
FR_CSOURCES += fs/fat/legacy/option/unicode.c
endif

ifeq (${CONFIG_FR_APPS_CLI},y)
FR_CINCDIRS += apps/cli
FR_CSOURCES += apps/cli/app_cli.c
FR_CSOURCES += apps/cli/cli.c
endif
