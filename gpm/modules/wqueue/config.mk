
ifeq (${CONFIG_MODULE_WQUEUE},y)

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
ifneq (${CONFIG_LIBC_PTHREAD}${CONFIG_LIBC_SEMAPHORE},yy)
$(error CONFIG_MODULE_WQUEUE depend on CONFIG_LIBC_PTHREAD, CONFIG_LIBC_SEMAPHORE)
endif
endif

PROJ_CINCDIRS += ${GPMPATH}/modules/wqueue
CSOURCES += ${GPMPATH}/modules/wqueue/work_cancel.c
CSOURCES += ${GPMPATH}/modules/wqueue/work_lock.c
CSOURCES += ${GPMPATH}/modules/wqueue/work_queue.c
CSOURCES += ${GPMPATH}/modules/wqueue/work_thread.c
endif
