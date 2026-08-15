

ifeq (${CONFIG_MTD},y)

ifneq (${CONFIG_LIBC_VFS},y)
$(error CONFIG_MTD depend on CONFIG_LIBC_VFS)
endif
CSOURCES += ${GPMPATH}/drivers/mtd/ftl.c
CSOURCES += ${GPMPATH}/drivers/mtd/mtd_partition.c

# CONFIG_AT24XX_SIZE=2
# CONFIG_AT24XX_ADDR=0x50
# CONFIG_AT24XX_FREQUENCY=100000
# CONFIG_AT24XX_MTD_BLOCKSIZE=8
ifeq (${CONFIG_MTD_AT24XX},y)
CSOURCES += ${GPMPATH}/drivers/mtd/at24xx.c
endif

# CONFIG_W25_SPIMODE=SPIDEV_MODE3
# CONFIG_W25_SECTOR512
# CONFIG_MTD_BYTE_WRITE
ifeq (${CONFIG_MTD_W25},y)
CSOURCES += ${GPMPATH}/drivers/mtd/w25.c
endif

ifeq (${CONFIG_MTD_RAMTRON},y)
CSOURCES += ${GPMPATH}/drivers/mtd/ramtron.c
endif

endif # end with CONFIG_MTD


