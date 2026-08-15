
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

include ${GPMPATH}/modules/container/config.mk
endif
