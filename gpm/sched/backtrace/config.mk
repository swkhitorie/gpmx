
ifeq (${CONFIG_CMBACKTRACE},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/backtrace
CSOURCES += ${GPMPATH}/sched/backtrace/cm_backtrace.c
ifeq ($(PROJ_TC),gae)
ASMSOURCES += ${GPMPATH}/sched/backtrace/fault_handler/gcc/cmb_fault.S
endif
ifeq ($(PROJ_TC),armcc)
ASMSOURCES += ${GPMPATH}/sched/backtrace/fault_handler/keil/cmb_fault.S
endif
ifeq ($(PROJ_TC),armclang)
ASMSOURCES += ${GPMPATH}/sched/backtrace/fault_handler/keil/cmb_fault.S
endif
endif
