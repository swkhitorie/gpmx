
ifeq (${CONFIG_DRIVER_SPI},y)
CSOURCES += ${GPMPATH}/drivers/spi/spi.c
endif

ifeq (${CONFIG_DRIVER_QUADSPI},y)
CSOURCES += ${GPMPATH}/drivers/spi/qspi.c
endif

ifeq (${CONFIG_DRIVER_MMCSDSPI},y)
CSOURCES += ${GPMPATH}/drivers/spi/mmcsd_spi.c
endif
