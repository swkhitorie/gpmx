
ifneq (${CONFIG_POSIXRUN_ENABLE},y)
CSOURCES += ${GPMPATH}/libs/libc/syslog/lib_setlogmask.c
CSOURCES += ${GPMPATH}/libs/libc/syslog/lib_syslog.c
endif
