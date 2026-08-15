
ifneq (${CONFIG_POSIXRUN_ENABLE},y)
CSOURCES += ${GPMPATH}/libs/libc/string/lib_strdup.c
CSOURCES += ${GPMPATH}/libs/libc/string/lib_strlcpy.c
endif
