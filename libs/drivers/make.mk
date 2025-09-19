
PROJ_CINCDIRS   +=  libs/drivers

# default simple device driver interface
CSOURCES   +=  libs/drivers/dnode.c
CSOURCES   +=  libs/drivers/rbdrv.c

CSOURCES   +=  libs/drivers/serial.c
CSOURCES   +=  libs/drivers/i2c_master.c
CSOURCES   +=  libs/drivers/spi.c
CSOURCES   +=  libs/drivers/can.c

CSOURCES   +=  libs/drivers/qspi.c
CSOURCES   +=  libs/drivers/sdio.c
CSOURCES   +=  libs/drivers/mmcsd.c
