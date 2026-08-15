
ifeq (${GPMPATH},)
$(error Invalid empty relpath gpm)
endif

ifeq (${GPMPATHABS},)
$(error Invalid empty abspath gpm)
endif

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
PROJ_CINCDIRS += ${GPMPATH}/include
endif

include ${GPMPATH}/sched/config.mk
include ${GPMPATH}/drivers/config.mk
include ${GPMPATH}/fs/config.mk
include ${GPMPATH}/mm/config.mk
include ${GPMPATH}/modules/config.mk
include ${GPMPATH}/libs/config.mk
include ${GPMPATH}/net/config.mk
include ${GPMPATH}/usb/config.mk
include ${GPMPATH}/tests/config.mk
