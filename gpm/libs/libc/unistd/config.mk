
ifeq (${CONFIG_LIBC_PTHREAD},y)
CSOURCES += ${GPMPATH}/libs/libc/unistd/lib_umask.c
endif

ifeq (${CONFIG_PIPES},y)
CSOURCES += ${GPMPATH}/libs/libc/unistd/lib_pipe.c
CSOURCES += ${GPMPATH}/libs/libc/unistd/lib_pipe2.c
endif
