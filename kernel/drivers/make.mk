PROJ_CINCDIRS   +=  kernel/drivers

ifeq (${MK_USE_HARDFAULTINFO},y)
PROJ_CINCDIRS   +=  kernel/drivers/hardfault_info
CSOURCES   +=  kernel/drivers/hardfault_info/cm_backtrace.c
ifeq ($(PROJ_TC),gae)
ASMSOURCES += kernel/drivers/hardfault_info/fault_handler/gcc/cmb_fault.s
endif
ifeq ($(PROJ_TC),armcc)
ASMSOURCES += kernel/drivers/hardfault_info/fault_handler/keil/cmb_fault.s
endif
ifeq ($(PROJ_TC),armclang)
ASMSOURCES += kernel/drivers/hardfault_info/fault_handler/keil/cmb_fault.s
endif
endif

ifeq (${MK_MEM_METHOD},fr4)
PROJ_CINCDIRS += kernel/drivers/mem
CSOURCES += kernel/drivers/mem/mem_fr4.c
endif

ifeq (${MK_MEM_METHOD},rtx)
PROJ_CINCDIRS += kernel/drivers/mem
CSOURCES += kernel/drivers/mem/mem_rtx.c
endif

ifeq (${MK_GPDRIVE_RINGBUFFER},y)
CSOURCES   +=  kernel/drivers/gringbuffer.c
endif # end with MK_GPDRIVE_RINGBUFFER

ifeq (${MK_GPDRIVE_RTC},y)
CSOURCES   +=  kernel/drivers/rtc.c
endif # end with MK_GPDRIVE_RTC

ifeq (${MK_GPDRIVE_DNODE},y)
CSOURCES   +=  kernel/drivers/dnode.c

ifeq (${MK_GPDRIVE_SERIAL},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_SERIAL
CSOURCES   +=  kernel/drivers/serial.c
endif
ifeq (${MK_GPDRIVE_I2C},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_I2C
CSOURCES   +=  kernel/drivers/i2c_master.c
endif
ifeq (${MK_GPDRIVE_SPI},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_SPI
CSOURCES   +=  kernel/drivers/spi.c
endif
ifeq (${MK_GPDRIVE_CAN},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_CAN
CSOURCES   +=  kernel/drivers/can.c
endif
ifeq (${MK_GPDRIVE_QUADSPI},y)
PROJ_CDEFS +=  CONFIG_GPDRIVE_QUADSPI
CSOURCES   +=  kernel/drivers/qspi.c
endif

endif # end with MK_GPDRIVE_DNODE
