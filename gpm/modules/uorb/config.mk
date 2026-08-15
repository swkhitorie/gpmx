
ifeq (${CONFIG_MODULE_UORB},y)

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
ifneq (${CONFIG_MODULE_WORKQUEUE}${CONFIG_LIBC_PTHREAD}${CONFIG_LIBC_SEMAPHORE},yyy)
$(error CONFIG_MODULE_UORB depend on CONFIG_MODULE_WORKQUEUE, CONFIG_LIBC_PTHREAD, CONFIG_LIBC_SEMAPHORE)
endif
else
ifneq (${CONFIG_MODULE_WORKQUEUE},y)
$(error CONFIG_MODULE_UORB depend on CONFIG_MODULE_WORKQUEUE)
endif
endif

PROJ_CINCDIRS += ${GPMPATH}/modules/uorb
CPPSOURCES += ${GPMPATH}/modules/uorb/Subscription.cpp
CSOURCES += ${GPMPATH}/modules/uorb/uorb_device_master.c
CSOURCES += ${GPMPATH}/modules/uorb/uorb_device_node.c
CSOURCES += ${GPMPATH}/modules/uorb/uorb_gnode.c
CSOURCES += ${GPMPATH}/modules/uorb/uorb_manager.c
CSOURCES += ${GPMPATH}/modules/uorb/uorb.c

# uorb include and source
UORB_CINCDIRSS = $(subst ;,,$(UORB_INCLUDING))
PROJ_CINCDIRS += $(subst ${SDK_ROOTDIR},,$(subst ;, ,$(UORB_INCLUDING)))
CPPSOURCES += $(subst ${SDK_ROOTDIR},,$(wildcard ${UORB_CINCDIRSS}/msg/topics_sources/*cpp))
endif
