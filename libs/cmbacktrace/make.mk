
ifeq (${MK_LIB_CMBACKTRACE},y)
PROJ_CINCDIRS   +=  libs/cmbacktrace
CSOURCES   +=  libs/cmbacktrace/cm_backtrace.c
ifeq ($(PROJ_TC),gae)
ASMSOURCES += libs/cmbacktrace/fault_handler/gcc/cmb_fault.s
endif
ifeq ($(PROJ_TC),armcc)
ASMSOURCES += libs/cmbacktrace/fault_handler/keil/cmb_fault.s
endif
ifeq ($(PROJ_TC),armclang)
ASMSOURCES += libs/cmbacktrace/fault_handler/keil/cmb_fault.s
endif
endif

