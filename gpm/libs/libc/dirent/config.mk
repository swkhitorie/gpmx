
ifeq (${CONFIG_LIBC_VFS},y)
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_closedir.c
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_dirfd.c
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_opendir.c
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_readdir.c
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_readdirr.c
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_rewinddir.c
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_seekdir.c
CSOURCES += ${GPMPATH}/libs/libc/dirent/lib_telldir.c
endif
