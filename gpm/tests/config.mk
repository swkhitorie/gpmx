ifeq (${CONFIG_TEST},y)
include ${GPMPATH}/tests/hrt/config.mk
include ${GPMPATH}/tests/usrtest/config.mk
include ${GPMPATH}/tests/libc/config.mk

include ${GPMPATH}/tests/modules_perf/config.mk
include ${GPMPATH}/tests/modules_uorb/config.mk
include ${GPMPATH}/tests/modules_workqueue/config.mk
include ${GPMPATH}/tests/modules_wqueue/config.mk
include ${GPMPATH}/tests/modules_nsh/config.mk

include ${GPMPATH}/tests/fs_fatfs/config.mk
include ${GPMPATH}/tests/fs_lfs/config.mk
include ${GPMPATH}/tests/fs_gromfs/config.mk

include ${GPMPATH}/tests/vfs_pipes/config.mk
include ${GPMPATH}/tests/vfs_mtd/config.mk
include ${GPMPATH}/tests/vfs_fs/config.mk
endif
