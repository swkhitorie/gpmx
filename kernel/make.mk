
ifeq (${MK_RTOS},frtos)
include ${SDK_ROOTDIR}/kernel/freertos/make.mk
endif

ifeq (${MK_RTOS},rt_nano)
include ${SDK_ROOTDIR}/kernel/rt_nano/make.mk
endif
