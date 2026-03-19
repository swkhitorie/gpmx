
PROJ_CINCDIRS += kernel/rttnano/include

ifeq (${MOD_ARCH},m7)
RT_PLATFORM := cortex-m7
else ifeq (${MOD_ARCH},m4)
RT_PLATFORM := cortex-m4
else ifeq (${MOD_ARCH},m3)
RT_PLATFORM := cortex-m3
else ifeq (${MOD_ARCH},m0)
RT_PLATFORM := cortex-m0
endif

PROJ_CINCDIRS += kernel/rttnano/libcpu/arm/${RT_PLATFORM}/
CSOURCES += kernel/rttnano/libcpu/arm/${RT_PLATFORM}/cpuport.c
ASMSOURCES += kernel/rttnano/libcpu/arm/${RT_PLATFORM}/context_${MK_RTOS_PLATFORM}.S

CSOURCES += kernel/rttnano/clock.c
CSOURCES += kernel/rttnano/components.c
CSOURCES += kernel/rttnano/cpu.c
CSOURCES += kernel/rttnano/device.c
CSOURCES += kernel/rttnano/idle.c
CSOURCES += kernel/rttnano/ipc.c
CSOURCES += kernel/rttnano/irq.c
CSOURCES += kernel/rttnano/kservice.c
CSOURCES += kernel/rttnano/mem.c
CSOURCES += kernel/rttnano/memheap.c
CSOURCES += kernel/rttnano/mempool.c
CSOURCES += kernel/rttnano/object.c
CSOURCES += kernel/rttnano/kerneluler.c
CSOURCES += kernel/rttnano/signal.c
CSOURCES += kernel/rttnano/slab.c
CSOURCES += kernel/rttnano/thread.c
CSOURCES += kernel/rttnano/timer.c

