ifeq (${MK_USE_KERNEL_POSIX_TIME},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_TIME
CSOURCES += ${GPMPATH}/libs/libc/time/clock.c
CSOURCES += ${GPMPATH}/libs/libc/time/clock_getres.c
CSOURCES += ${GPMPATH}/libs/libc/time/clock_gettime.c
CSOURCES += ${GPMPATH}/libs/libc/time/clock_nanosleep.c
CSOURCES += ${GPMPATH}/libs/libc/time/clock_settime.c
CSOURCES += ${GPMPATH}/libs/libc/time/lib_nanosleep.c
CSOURCES += ${GPMPATH}/libs/libc/unistd/lib_sleep.c
CSOURCES += ${GPMPATH}/libs/libc/utils/utils.c
endif
