ifeq (${MK_USE_KERNEL_WORKQUEUE},y)
PROJ_CDEFS += CONFIG_MODULE_WORKQUEUE
PROJ_CINCDIRS += ${GPMPATH}/modules/common
PROJ_CINCDIRS += ${GPMPATH}/modules/ccontainer
PROJ_CINCDIRS += ${GPMPATH}/modules/workqueue
CSOURCES += ${GPMPATH}/modules/ccontainer/blocking_list.c
CSOURCES += ${GPMPATH}/modules/ccontainer/intrusive_queue.c
CSOURCES += ${GPMPATH}/modules/ccontainer/intrusive_list.c
CSOURCES += ${GPMPATH}/modules/ccontainer/intrusive_sorted_list.c
CSOURCES += ${GPMPATH}/modules/workqueue/scheduledworkitem.c
CSOURCES += ${GPMPATH}/modules/workqueue/workitem.c
CSOURCES += ${GPMPATH}/modules/workqueue/workqueue.c
CSOURCES += ${GPMPATH}/modules/workqueue/workqueue_manager.c

MK_USE_KERNEL_POSIX_PTHREAD:=y
MK_USE_KERNEL_POSIX_SEMAPHORE:=y
endif
