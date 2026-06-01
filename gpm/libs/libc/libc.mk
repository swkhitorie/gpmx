
include ${GPMPATH}/libs/libc/pthread/config.mk
include ${GPMPATH}/libs/libc/sched/config.mk
include ${GPMPATH}/libs/libc/mqueue/config.mk
include ${GPMPATH}/libs/libc/queue/config.mk
include ${GPMPATH}/libs/libc/semaphore/config.mk
include ${GPMPATH}/libs/libc/timer/config.mk
include ${GPMPATH}/libs/libc/time/config.mk
include ${GPMPATH}/libs/libc/libgen/config.mk

CSOURCES += ${GPMPATH}/libs/libc/utils/compile_utils.c
