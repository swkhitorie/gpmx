
ifeq (${CONFIG_FS_LITTLEFS},y)
CSOURCES += ${GPMPATH}/tests/fs_lfs/lfs_test.c
endif
