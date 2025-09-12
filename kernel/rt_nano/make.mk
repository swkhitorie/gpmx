
PROJ_CINCDIRS += kernel/rt_nano/include

ifeq (${MOD_ARCH},m7)
RT_PLATFORM := cortex-m7
else ifeq (${MOD_ARCH},m4)
RT_PLATFORM := cortex-m4
else ifeq (${MOD_ARCH},m3)
RT_PLATFORM := cortex-m3
else ifeq (${MOD_ARCH},m0)
RT_PLATFORM := cortex-m0
endif

PROJ_CINCDIRS += kernel/rt_nano/libcpu/arm/${RT_PLATFORM}/
CSOURCES += kernel/rt_nano/libcpu/arm/${RT_PLATFORM}/cpuport.c
ASMSOURCES += kernel/rt_nano/libcpu/arm/${RT_PLATFORM}/context_${MK_RTOS_PLATFORM}.S

CSOURCES += kernel/rt_nano/clock.c
CSOURCES += kernel/rt_nano/components.c
CSOURCES += kernel/rt_nano/cpu.c
CSOURCES += kernel/rt_nano/device.c
CSOURCES += kernel/rt_nano/idle.c
CSOURCES += kernel/rt_nano/ipc.c
CSOURCES += kernel/rt_nano/irq.c
CSOURCES += kernel/rt_nano/kservice.c
CSOURCES += kernel/rt_nano/mem.c
CSOURCES += kernel/rt_nano/memheap.c
CSOURCES += kernel/rt_nano/mempool.c
CSOURCES += kernel/rt_nano/object.c
CSOURCES += kernel/rt_nano/kerneluler.c
CSOURCES += kernel/rt_nano/signal.c
CSOURCES += kernel/rt_nano/slab.c
CSOURCES += kernel/rt_nano/thread.c
CSOURCES += kernel/rt_nano/timer.c

