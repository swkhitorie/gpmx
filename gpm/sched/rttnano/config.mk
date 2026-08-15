
ifeq (${CONFIG_RTTNANO_ENABLE},y)

ifeq (${CONFIG_CMBACKTRACE},y)
$(error CONFIG_RTTNANO_ENABLE can no support CONFIG_CMBACKTRACE)
endif

PROJ_CINCDIRS += ${GPMPATH}/sched/rttnano/include

ifeq (${MOD_ARCH},m7)
RT_PLATFORM := cortex-m7
else ifeq (${MOD_ARCH},m4)
RT_PLATFORM := cortex-m4
else ifeq (${MOD_ARCH},m3)
RT_PLATFORM := cortex-m3
else ifeq (${MOD_ARCH},m0)
RT_PLATFORM := cortex-m0
endif

PROJ_CINCDIRS += ${GPMPATH}/sched/rttnano/libcpu/arm/${RT_PLATFORM}/
CSOURCES += ${GPMPATH}/sched/rttnano/libcpu/arm/${RT_PLATFORM}/cpuport.c

ifeq (${CONFIG_RTOS_COMPILER},"gcc")
ASMSOURCES += ${GPMPATH}/sched/rttnano/libcpu/arm/${RT_PLATFORM}/context_gcc.S
else ifeq (${CONFIG_RTOS_COMPILER},"rvds")
ASMSOURCES += ${GPMPATH}/sched/rttnano/libcpu/arm/${RT_PLATFORM}/context_rvds.S
endif

CSOURCES += ${GPMPATH}/sched/rttnano/clock.c
CSOURCES += ${GPMPATH}/sched/rttnano/components.c
CSOURCES += ${GPMPATH}/sched/rttnano/cpu.c
CSOURCES += ${GPMPATH}/sched/rttnano/device.c
CSOURCES += ${GPMPATH}/sched/rttnano/idle.c
CSOURCES += ${GPMPATH}/sched/rttnano/ipc.c
CSOURCES += ${GPMPATH}/sched/rttnano/irq.c
CSOURCES += ${GPMPATH}/sched/rttnano/kservice.c
CSOURCES += ${GPMPATH}/sched/rttnano/mem.c
CSOURCES += ${GPMPATH}/sched/rttnano/memheap.c
CSOURCES += ${GPMPATH}/sched/rttnano/mempool.c
CSOURCES += ${GPMPATH}/sched/rttnano/object.c
CSOURCES += ${GPMPATH}/sched/rttnano/scheduler.c
CSOURCES += ${GPMPATH}/sched/rttnano/signal.c
CSOURCES += ${GPMPATH}/sched/rttnano/slab.c
CSOURCES += ${GPMPATH}/sched/rttnano/thread.c
CSOURCES += ${GPMPATH}/sched/rttnano/timer.c

ifeq (${CONFIG_FINSH},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/rttnano/finsh
CSOURCES += ${GPMPATH}/sched/rttnano/finsh/cmd.c
CSOURCES += ${GPMPATH}/sched/rttnano/finsh/msh_file.c
CSOURCES += ${GPMPATH}/sched/rttnano/finsh/msh_parse.c
CSOURCES += ${GPMPATH}/sched/rttnano/finsh/msh.c
CSOURCES += ${GPMPATH}/sched/rttnano/finsh/shell.c
endif

include ${GPMPATH}/sched/rttnano/test/config.mk

endif
