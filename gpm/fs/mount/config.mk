
ifeq (${CONFIG_LIBC_VFS},y)
CSOURCES += ${GPMPATH}/fs/mount/fs_mount.c
CSOURCES += ${GPMPATH}/fs/mount/fs_umount2.c
endif
