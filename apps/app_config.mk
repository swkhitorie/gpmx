
ifeq (${APP_SUBPATH},)
$(error Invalid empty APP_SUBPATH)
endif

include ${SDK_ROOTDIR}/apps/${APP_SUBPATH}/config.mk

include ${SDK_ROOTDIR}/kernel/make.mk

include ${SDK_ROOTDIR}/libs/make.mk
