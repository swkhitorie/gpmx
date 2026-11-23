
PROJ_NAME   :=  gnss_ins
APP_PROJ_DIR =  apps/gnss_ins
PROJ_TC    := gcc
BUILD_TYPE := app

ifeq ($(filter build,$(MAKECMDGOALS)),build)
include ${SDK_ROOTDIR}/${APP_PROJ_DIR}/define.mk
endif
