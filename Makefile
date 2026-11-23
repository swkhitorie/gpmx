# Path to root dir
SDK_DIR_MK = $(dir $(realpath $(lastword ${MAKEFILE_LIST})))
SDK_DIR_CUR = $(CURRENT_DIR)

SDK_ROOTDIR := ${SDK_DIR_MK}

GPMPATH           := gpm
GPMPATHABS        := ${SDK_ROOTDIR}/${GPMPATH}
MAKEFILES_ROOTDIR := ${GPMPATHABS}/tools

ifneq ($(wildcard ${SDK_ROOTDIR}/apps/${APP_SUBPATH}/config.mk),)
include ${SDK_ROOTDIR}/apps/${APP_SUBPATH}/config.mk
include $(MAKEFILES_ROOTDIR)/make/build.mk
endif
