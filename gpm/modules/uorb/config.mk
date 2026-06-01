ifeq (${MK_USE_KERNEL_UORB},y)
PROJ_CDEFS += CONFIG_MODULE_UORB
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

MK_USE_KERNEL_WORKQUEUE=y
MK_USE_KERNEL_POSIX_PTHREAD:=y
MK_USE_KERNEL_POSIX_SEMAPHORE:=y
endif
