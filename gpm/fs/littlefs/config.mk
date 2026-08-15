
ifeq (${CONFIG_FS_LITTLEFS},y)
PROJ_CINCDIRS += ${GPMPATH}/fs/littlefs/
CSOURCES += ${GPMPATH}/fs/littlefs/lfs_util.c
CSOURCES += ${GPMPATH}/fs/littlefs/lfs.c

ifeq (${CONFIG_LIBC_VFS},y)
CSOURCES += ${GPMPATH}/fs/littlefs/lfs_vfs.c
endif
endif
