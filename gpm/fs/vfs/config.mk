
ifeq (${CONFIG_LIBC_VFS},y)
PROJ_CINCDIRS+=${GPMPATH}/fs/
CSOURCES += ${GPMPATH}/fs/fs_initialize.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_close.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_dir.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_dup.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_dup2.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_fcntl.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_fstat.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_fsync.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_ioctl.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_lseek.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_mkdir.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_open.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_poll.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_read.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_rename.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_rmdir.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_stat.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_statfs.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_truncate.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_unlink.c
CSOURCES += ${GPMPATH}/fs/vfs/fs_write.c
endif
