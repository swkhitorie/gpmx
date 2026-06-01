
CSOURCES   +=  ${GPMPATH}/drivers/gringbuffer.c
CSOURCES   +=  ${GPMPATH}/drivers/sched.c
CSOURCES   +=  ${GPMPATH}/drivers/rtc.c

ifeq (${MK_GPDRIVE_SERIAL},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_SERIAL
CSOURCES   +=  ${GPMPATH}/drivers/serial.c
endif
ifeq (${MK_GPDRIVE_I2C},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_I2C
CSOURCES   +=  ${GPMPATH}/drivers/i2c_master.c
endif
ifeq (${MK_GPDRIVE_SPI},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_SPI
CSOURCES   +=  ${GPMPATH}/drivers/spi.c
endif
ifeq (${MK_GPDRIVE_CAN},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_CAN
CSOURCES   +=  ${GPMPATH}/drivers/can.c
endif
ifeq (${MK_GPDRIVE_QUADSPI},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_QUADSPI
CSOURCES   +=  ${GPMPATH}/drivers/qspi.c
endif
ifeq (${MK_GPDRIVE_MMCSDSPI},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_MMCSDSPI
CSOURCES   +=  ${GPMPATH}/drivers/mmcsd_spi.c
endif
