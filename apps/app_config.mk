
ifeq (${APP_SUBPATH},)
ifeq (${MAKE_TARGET_CLEANS},n)
$(error Invalid empty APP_SUBPATH)
endif
endif

include ${SDK_ROOTDIR}/apps/${APP_SUBPATH}/config.mk

include ${SDK_ROOTDIR}/include/make.mk

include ${SDK_ROOTDIR}/kernel/make.mk

include ${SDK_ROOTDIR}/libs/make.mk

include ${SDK_ROOTDIR}/px4/make.mk
