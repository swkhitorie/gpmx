PROJ_CINCDIRS   +=  libs/drivers

ifeq (${MK_GPDRIVE_RINGBUFFER},y)
CSOURCES   +=  libs/drivers/gringbuffer.c
endif # end with MK_GPDRIVE_RINGBUFFER

ifeq (${MK_GPDRIVE_RTC},y)
CSOURCES   +=  libs/drivers/rtc.c
endif # end with MK_GPDRIVE_RTC

ifeq (${MK_GPDRIVE_DNODE},y)
CSOURCES   +=  libs/drivers/dnode.c

ifeq (${MK_GPDRIVE_SERIAL},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_SERIAL
CSOURCES   +=  libs/drivers/serial.c
endif
ifeq (${MK_GPDRIVE_I2C},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_I2C
CSOURCES   +=  libs/drivers/i2c_master.c
endif
ifeq (${MK_GPDRIVE_SPI},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_SPI
CSOURCES   +=  libs/drivers/spi.c
endif
ifeq (${MK_GPDRIVE_CAN},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_CAN
CSOURCES   +=  libs/drivers/can.c
endif
ifeq (${MK_GPDRIVE_QUADSPI},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_QUADSPI
CSOURCES   +=  libs/drivers/qspi.c
endif

endif # end with MK_GPDRIVE_DNODE
