
include ${SDK_ROOTDIR}/kernel/drivers/make.mk

include ${SDK_ROOTDIR}/kernel/module/make.mk

ifeq (${MK_RTOS},frtos)
PROJ_CDEFS += CONFIG_FREERTOS_ENABLE
include ${SDK_ROOTDIR}/kernel/freertos/make.mk
endif

ifeq (${MK_RTOS},rttnano)
PROJ_CDEFS += CONFIG_RTTNANO_ENABLE
include ${SDK_ROOTDIR}/kernel/rttnano/make.mk
endif
