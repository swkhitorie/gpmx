
ifeq (${CONFIG_LIBC_VFS},y)
CSOURCES += ${GPMPATH}/tests/vfs_fs/vfs_fs_test.c
endif

ifeq (${CONFIG_FS_CROMFS},y)
# CSOURCES += ${GPMPATH}/tests/vfs_fs/gencromfs.c
endif
