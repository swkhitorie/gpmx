
GPMPATH:=gpm
GPMPATHABS:=${SDK_ROOTDIR}/${GPMPATH}

include ${GPMPATH}/drivers/config.mk
include ${GPMPATH}/fs/config.mk
include ${GPMPATH}/include/config.mk
include ${GPMPATH}/sched/config.mk
include ${GPMPATH}/mm/config.mk
include ${GPMPATH}/modules/config.mk
include ${GPMPATH}/libs/config.mk
include ${GPMPATH}/net/config.mk
include ${GPMPATH}/usb/config.mk
include ${GPMPATH}/tests/config.mk

include ${GPMPATH}/modules/fs/make.mk

