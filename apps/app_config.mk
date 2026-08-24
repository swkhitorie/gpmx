
ifeq (${APP_SUBPATH},)
$(error Invalid empty APP_SUBPATH)
endif

ifeq (${BUILD_USR_BOARD},)
$(error Invalid empty BUILD_USR_BOARD)
endif

include ${SDK_ROOTDIR}/apps/${APP_SUBPATH}/config.mk
PROJ_CDEFS += $(foreach v, $(sort $(filter CONFIG_% , $(.VARIABLES))), $v=$($v))

ifneq ($(BUILD_USR_BOARD),unix)

BOARD_MAKEFILE := ${SDK_ROOTDIR}/boards/${BUILD_USR_BOARD}/make.mk
ifeq ($(wildcard $(BOARD_MAKEFILE)),)
$(error Board makefile not found: $(BOARD_MAKEFILE))
endif
include $(BOARD_MAKEFILE)

endif

include ${SDK_ROOTDIR}/gpm/config.mk
