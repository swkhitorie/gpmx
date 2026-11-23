
CPPSOURCES += ${PX4LIBPATH}/lib/platform/module.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/platform/px4_log.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/platform/px4_task.cpp
CSOURCES += ${PX4LIBPATH}/lib/platform/px4_getopt.c

ifeq (${CONFIG_POSIXRUN_ENABLE}|${CONFIG_HRT},y|y)
CPPSOURCES += ${PX4LIBPATH}/lib/platform/lockstep_scheduler/lockstep_components.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/platform/lockstep_scheduler/lockstep_scheduler.cpp
CPPSOURCES += ${PX4LIBPATH}/lib/platform/drv_hrt.cpp
endif
