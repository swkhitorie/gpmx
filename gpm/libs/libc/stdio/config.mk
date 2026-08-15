
ifneq (${CONFIG_POSIXRUN_ENABLE},y)

ifeq (${CONFIG_KPRINTF},y)
PROJ_CINCDIRS += ${GPMPATH}/libs/libc/stdio
CSOURCES += ${GPMPATH}/libs/libc/stdio/lib_kprintf.c
endif
CSOURCES += ${GPMPATH}/libs/libc/stdio/lib_asprintf.c
CSOURCES += ${GPMPATH}/libs/libc/stdio/lib_vasprintf.c

endif
