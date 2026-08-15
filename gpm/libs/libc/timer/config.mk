ifeq (${CONFIG_LIBC_TIMER},y)
CSOURCES += ${GPMPATH}/libs/libc/timer/prv_timer.c
CSOURCES += ${GPMPATH}/libs/libc/timer/timer_create.c
CSOURCES += ${GPMPATH}/libs/libc/timer/timer_delete.c
CSOURCES += ${GPMPATH}/libs/libc/timer/timer_getoverrun.c
CSOURCES += ${GPMPATH}/libs/libc/timer/timer_gettime.c
CSOURCES += ${GPMPATH}/libs/libc/timer/timer_settime.c

MK_USE_KERNEL_POSIX_TIME:=y
endif
