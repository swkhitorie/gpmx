
ifeq (${APP_SUBPATH},)
$(error Invalid empty APP_SUBPATH)
endif

include ${SDK_ROOTDIR}/apps/${APP_SUBPATH}/config.mk
PROJ_CDEFS += $(foreach v, $(sort $(filter CONFIG_% , $(.VARIABLES))), $v=$($v))

include ${SDK_ROOTDIR}/apps/board_selection.mk
include ${SDK_ROOTDIR}/gpm/config.mk
