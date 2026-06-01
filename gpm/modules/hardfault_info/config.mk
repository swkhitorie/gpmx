
ifeq (${MK_USE_HARDFAULTINFO},y)
PROJ_CDEFS += CONFIG_MODULE_CMBACKTRACE
PROJ_CINCDIRS += ${GPMPATH}/modules/hardfault_info
CSOURCES += ${GPMPATH}/modules/hardfault_info/cm_backtrace.c
CSOURCES += ${GPMPATH}/modules/hardfault_info/fault_test.c
ifeq ($(PROJ_TC),gae)
ASMSOURCES += ${GPMPATH}/modules/hardfault_info/fault_handler/gcc/cmb_fault.s
endif
ifeq ($(PROJ_TC),armcc)
ASMSOURCES += ${GPMPATH}/modules/hardfault_info/fault_handler/keil/cmb_fault.s
endif
ifeq ($(PROJ_TC),armclang)
ASMSOURCES += ${GPMPATH}/modules/hardfault_info/fault_handler/keil/cmb_fault.s
endif
endif
