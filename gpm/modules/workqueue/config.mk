
ifeq (${CONFIG_MODULE_WORKQUEUE},y)

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
ifneq (${CONFIG_LIBC_PTHREAD}${CONFIG_LIBC_SEMAPHORE},yy)
$(error CONFIG_MODULE_WORKQUEUE depend on CONFIG_LIBC_PTHREAD, CONFIG_LIBC_SEMAPHORE)
endif
endif

PROJ_CINCDIRS += ${GPMPATH}/modules/workqueue
CSOURCES += ${GPMPATH}/modules/workqueue/scheduledworkitem.c
CSOURCES += ${GPMPATH}/modules/workqueue/workitem.c
CSOURCES += ${GPMPATH}/modules/workqueue/workqueue.c
CSOURCES += ${GPMPATH}/modules/workqueue/workqueue_manager.c

CPPSOURCES += ${GPMPATH}/modules/workqueue/ScheduledWorkItem.cpp
CPPSOURCES += ${GPMPATH}/modules/workqueue/WorkItem.cpp
CPPSOURCES += ${GPMPATH}/modules/workqueue/WorkItemSingleShot.cpp

CSOURCES += ${GPMPATH}/modules/workqueue/cmd.c

ifeq (${CONFIG_MODULE_NSHLIB},y)
MODULE_LIBRARIES += workqueue_built
MOD_MAIN_workqueue_built  := workqueue
MOD_PRIO_workqueue_built  := 16
MOD_STACK_workqueue_built := 1024
endif

include ${GPMPATH}/modules/container/config.mk
endif
