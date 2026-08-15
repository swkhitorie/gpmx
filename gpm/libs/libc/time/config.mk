ifeq (${CONFIG_LIBC_TIME},y)
CSOURCES += ${GPMPATH}/libs/libc/time/clock_getres.c
CSOURCES += ${GPMPATH}/libs/libc/time/clock_gettime.c
CSOURCES += ${GPMPATH}/libs/libc/time/clock_nanosleep.c
CSOURCES += ${GPMPATH}/libs/libc/time/clock_settime.c
CSOURCES += ${GPMPATH}/libs/libc/time/lib_clock.c
CSOURCES += ${GPMPATH}/libs/libc/time/lib_sleep.c
CSOURCES += ${GPMPATH}/libs/libc/time/lib_nanosleep.c
CSOURCES += ${GPMPATH}/libs/libc/time/utils.c
endif
