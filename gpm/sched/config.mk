
ifeq (${MK_RTOS},frtos)
PROJ_CDEFS += CONFIG_FREERTOS_ENABLE
include ${GPMPATH}/sched/freertos/config.mk
endif

ifeq (${MK_RTOS},rttnano)
PROJ_CDEFS += CONFIG_RTTNANO_ENABLE
include ${GPMPATH}/sched/rttnano/config.mk
endif
