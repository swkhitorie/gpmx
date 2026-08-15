
ifeq (${CONFIG_LIBC_VFS},y)
CSOURCES += ${GPMPATH}/libs/libc/libgen/lib_basename.c
CSOURCES += ${GPMPATH}/libs/libc/libgen/lib_dirname.c
endif
