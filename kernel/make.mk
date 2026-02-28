
include ${SDK_ROOTDIR}/kernel/drivers/make.mk

include ${SDK_ROOTDIR}/kernel/module/make.mk

ifeq (${MK_RTOS},frtos)
PROJ_CDEFS += CONFIG_FREERTOS_ENABLE
include ${SDK_ROOTDIR}/kernel/freertos/make.mk
endif

ifeq (${MK_RTOS},rt_nano)
PROJ_CDEFS += CONFIG_RTTHREADNANO_ENABLE
include ${SDK_ROOTDIR}/kernel/rt_nano/make.mk
endif
