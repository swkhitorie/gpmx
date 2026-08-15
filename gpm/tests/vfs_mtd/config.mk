
ifeq (${CONFIG_MTD},y)

ifeq (${CONFIG_MTD_AT24XX},y)
CSOURCES += ${GPMPATH}/tests/vfs_mtd/vfs_mtd_at24xx_test.c
endif

ifeq (${CONFIG_MTD_W25},y)
CSOURCES += ${GPMPATH}/tests/vfs_mtd/vfs_mtd_w25_test.c
endif

ifeq (${CONFIG_MTD_RAMTRON},y)
CSOURCES += ${GPMPATH}/tests/vfs_mtd/vfs_mtd_ramtron_test.c
endif

endif
