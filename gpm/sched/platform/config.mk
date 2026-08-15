
PROJ_CINCDIRS += ${GPMPATH}/sched/platform/
CSOURCES += ${GPMPATH}/sched/platform/drv_sched.c
CSOURCES += ${GPMPATH}/sched/platform/drv_grb.c

ifeq (${CONFIG_HRT},y)
ifeq (${CONFIG_POSIXRUN_ENABLE},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/platform/hrt/queue
CSOURCES += ${GPMPATH}/sched/platform/hrt/hrt_empty.c
else
CSOURCES += ${GPMPATH}/sched/platform/hrt/hrt_stm32.c
endif
endif
