

ifeq (${CONFIG_FR_POSIX},y)
FR_LIB_CSOURCES += $(subst ${FR_ROOTDIR}/,,$(wildcard ${FR_ROOTDIR}/libs/mqueue/*c))
FR_LIB_CSOURCES += $(subst ${FR_ROOTDIR}/,,$(wildcard ${FR_ROOTDIR}/libs/pthread/*c))
FR_LIB_CSOURCES += $(subst ${FR_ROOTDIR}/,,$(wildcard ${FR_ROOTDIR}/libs/queue/*c))
FR_LIB_CSOURCES += $(subst ${FR_ROOTDIR}/,,$(wildcard ${FR_ROOTDIR}/libs/semaphore/*c))
FR_LIB_CSOURCES += $(subst ${FR_ROOTDIR}/,,$(wildcard ${FR_ROOTDIR}/libs/time/*c))
FR_LIB_CSOURCES += $(subst ${FR_ROOTDIR}/,,$(wildcard ${FR_ROOTDIR}/libs/timer/*c))
FR_LIB_CSOURCES += $(subst ${FR_ROOTDIR}/,,$(wildcard ${FR_ROOTDIR}/libs/unistd/*c))
FR_LIB_CSOURCES += utils.c
endif

