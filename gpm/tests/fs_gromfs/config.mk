
ifeq (${CONFIG_GROMFS_ENABLE},y)
CSOURCES += ${GPMPATH}/tests/fs_gromfs/romfs_test_data.c
CSOURCES += ${GPMPATH}/tests/fs_gromfs/romfs_test.c
endif
