
ifeq (${MK_RTOS},frtos)
include ${SDK_ROOTDIR}/kernel/freertos/make.mk
endif

ifeq (${MK_RTOS},rt_nano)
include ${SDK_ROOTDIR}/kernel/rt_nano/make.mk
endif

ifeq (${MK_MEM_METHOD},fr4)
PROJ_CINCDIRS += kernel/rmem
CSOURCES += kernel/rmem/mem_fr4.c
endif

ifeq (${MK_MEM_METHOD},rtx)
PROJ_CINCDIRS += kernel/rmem
CSOURCES += kernel/rmem/mem_rtx.c
endif
