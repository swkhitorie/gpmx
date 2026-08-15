
ifeq (${CONFIG_CMBACKTRACE},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/platform/backtrace
CSOURCES += ${GPMPATH}/sched/platform/backtrace/cm_backtrace.c
ifeq ($(PROJ_TC),gae)
ASMSOURCES += ${GPMPATH}/sched/platform/backtrace/fault_handler/gcc/cmb_fault.S
endif
ifeq ($(PROJ_TC),armcc)
ASMSOURCES += ${GPMPATH}/sched/platform/backtrace/fault_handler/keil/cmb_fault.S
endif
ifeq ($(PROJ_TC),armclang)
ASMSOURCES += ${GPMPATH}/sched/platform/backtrace/fault_handler/keil/cmb_fault.S
endif
endif
