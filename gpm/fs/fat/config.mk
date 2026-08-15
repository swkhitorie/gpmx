
ifeq (${CONFIG_FS_FATFS},y)
PROJ_CINCDIRS += ${GPMPATH}/fs/fat/
CSOURCES += ${GPMPATH}/fs/fat/ff.c
CSOURCES += ${GPMPATH}/fs/fat/diskio.c
CSOURCES += ${GPMPATH}/fs/fat/ff_drv.c
CSOURCES += ${GPMPATH}/fs/fat/ffsystem.c
CSOURCES += ${GPMPATH}/fs/fat/ffunicode.c

ifeq (${CONFIG_LIBC_VFS},y)
CSOURCES += ${GPMPATH}/fs/fat/fs_fatfs_vfs.c
endif
endif
