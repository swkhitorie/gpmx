
PROJ_CINCDIRS += ${GPMPATH}/sched/platform/
CSOURCES += ${GPMPATH}/sched/platform/drv_sched.c
CSOURCES += ${GPMPATH}/sched/platform/drv_grb.c

ifeq (${CONFIG_HRT},y)
ifeq (${CONFIG_POSIXRUN_ENABLE},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/platform/hrt/queue
endif
ifeq (${CONFIG_HRT_STM32},y)
CSOURCES += ${GPMPATH}/sched/platform/hrt/hrt_stm32.c
else ifeq (${CONFIG_HRT_POSIX},y)
CPPSOURCES += ${GPMPATH}/sched/platform/hrt/hrt_posix.c
else ifeq (${CONFIG_HRT_EMPTY},y)
CSOURCES += ${GPMPATH}/sched/platform/hrt/hrt_empty.c
endif
ifeq (${CONFIG_HRT_POSIX2},y)
CSOURCES += ${GPMPATH}/modules/wqueue/hrt_queue.c
CSOURCES += ${GPMPATH}/modules/wqueue/hrt_thread.c
CSOURCES += ${GPMPATH}/modules/wqueue/hrt_work_cancel.c
endif
endif
