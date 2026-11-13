
PROJ_NAME    :=  boards_test
APP_PROJ_DIR :=  apps/boards_test

PROJ_TC      :=  gae
BUILD_TYPE   :=  firmware

ifeq ($(filter build,$(MAKECMDGOALS)),build)
ifneq ($(wildcard ${SDK_ROOTDIR}/boards/${APP_BUILT_BOARD}/make.mk),)
include ${SDK_ROOTDIR}/${APP_PROJ_DIR}/define.mk
include ${SDK_ROOTDIR}/boards/${APP_BUILT_BOARD}/make.mk
include ${SDK_ROOTDIR}/gpm/config.mk
endif
endif
