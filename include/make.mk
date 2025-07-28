
ifeq (${MK_RTOS},frtos)
PROJ_CINCDIRS += include/kernel/freertos
endif

ifeq (${MK_RTOS},rt_nano)
PROJ_CINCDIRS += include/kernel/rt_nano
endif

PROJ_CINCDIRS += include/

ifeq (${MK_USE_POSIX},y)
PROJ_CINCDIRS += include/posix
endif
